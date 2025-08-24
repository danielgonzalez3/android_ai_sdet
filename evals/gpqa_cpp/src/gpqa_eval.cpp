#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

struct GpqaItem {
	std::string id;
	std::string question;
	std::vector<std::string> choices; // size 4
	char answer; // 'A'..'D'
};

std::string trim(const std::string &s) {
	size_t start = 0;
	while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
	size_t end = s.size();
	while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
	return s.substr(start, end - start);
}

// csv reader
std::vector<std::string> parseCsvRow(const std::string &line) {
	std::vector<std::string> cells;
	std::string cell;
	bool in_quotes = false;
	for (size_t i = 0; i < line.size(); ++i) {
		char c = line[i];
		if (c == '"') {
			if (in_quotes && i + 1 < line.size() && line[i + 1] == '"') {
				cell.push_back('"');
				++i; // skip escaped quote
			} else {
				in_quotes = !in_quotes;
			}
		} else if (c == ',' && !in_quotes) {
			cells.push_back(cell);
			cell.clear();
		} else {
			cell.push_back(c);
		}
	}
	cells.push_back(cell);
	return cells;
}

// choices column is a JSON-like array string: ["opt1", "opt2", ...]
std::vector<std::string> parseChoicesArray(const std::string &raw) {
	std::vector<std::string> out;
	std::string s = trim(raw);
	if (s.empty()) return out;
	// Remove leading/trailing brackets if present
	if (s.front() == '[' && s.back() == ']') {
		s = s.substr(1, s.size() - 2);
	}
	bool in_quotes = false;
	std::string cur;
	for (size_t i = 0; i < s.size(); ++i) {
		char c = s[i];
		if (c == '"') {
			if (in_quotes && i + 1 < s.size() && s[i + 1] == '"') {
				cur.push_back('"');
				++i;
			} else {
				in_quotes = !in_quotes;
			}
		} else if (c == ',' && !in_quotes) {
			out.push_back(trim(cur));
			cur.clear();
		} else {
			cur.push_back(c);
		}
	}
	if (!cur.empty()) out.push_back(trim(cur));
	// Strip wrapping quotes on each element
	for (std::string &e : out) {
		std::string t = trim(e);
		if (!t.empty() && t.front() == '"' && t.back() == '"') {
			e = t.substr(1, t.size() - 2);
		} else {
			e = t;
		}
	}
	return out;
}

std::vector<GpqaItem> loadGpqaCsv(const std::string &path) {
	std::ifstream in(path);
	if (!in) throw std::runtime_error("Failed to open GPQA CSV: " + path);
	std::string header;
	std::getline(in, header);
	// Expect columns: id, subdomain, question, choices, answer
	std::vector<GpqaItem> items;
	std::string line;
	while (std::getline(in, line)) {
		if (line.empty()) continue;
		auto cells = parseCsvRow(line);
		if (cells.size() < 5) continue;
		GpqaItem it;
		it.id = trim(cells[0]);
		it.question = trim(cells[2]);
		it.choices = parseChoicesArray(cells[3]);
		std::string ans = trim(cells[4]);
		if (!ans.empty()) it.answer = static_cast<char>(std::toupper(static_cast<unsigned char>(ans[0])));
		items.push_back(std::move(it));
	}
	return items;
}

std::unordered_map<std::string, char> loadPredictionsCsv(const std::string &path) {
	std::ifstream in(path);
	if (!in) throw std::runtime_error("Failed to open predictions CSV: " + path);
	std::string header;
	std::getline(in, header);
	// Expect columns: id,choice (choice as A/B/C/D)
	std::unordered_map<std::string, char> pred;
	std::string line;
	while (std::getline(in, line)) {
		if (line.empty()) continue;
		auto cells = parseCsvRow(line);
		if (cells.size() < 2) continue;
		std::string id = trim(cells[0]);
		std::string ch = trim(cells[1]);
		if (!id.empty() && !ch.empty()) {
			pred[id] = static_cast<char>(std::toupper(static_cast<unsigned char>(ch[0])));
		}
	}
	return pred;
}

void writeRandomPredictions(const std::vector<GpqaItem> &items, const std::string &out_path, uint64_t seed) {
	std::mt19937_64 rng(seed);
	std::uniform_int_distribution<int> dist(0, 3);
	std::ofstream out(out_path);
	out << "id,choice\n";
	for (const auto &it : items) {
		char choice = static_cast<char>('A' + dist(rng));
		out << it.id << "," << choice << "\n";
	}
}

struct Metrics { double accuracy = 0.0; int correct = 0; int total = 0; };

Metrics evaluate(const std::vector<GpqaItem> &items, const std::unordered_map<std::string, char> &pred) {
	Metrics m{};
	m.total = static_cast<int>(items.size());
	for (const auto &it : items) {
		auto itp = pred.find(it.id);
		if (itp != pred.end() && itp->second == it.answer) {
			m.correct += 1;
		}
	}
	if (m.total > 0) m.accuracy = static_cast<double>(m.correct) / static_cast<double>(m.total);
	return m;
}

void printUsage(const char *argv0) {
	std::cerr << "Usage:\n"
	          << "  " << argv0 << " --data gpqa.csv --pred preds.csv\n"
	          << "  " << argv0 << " --data gpqa.csv --random --seed 42 --out preds_random.csv\n";
}

} // namespace

int main(int argc, char **argv) {
	std::string data_path;
	std::string pred_path;
	std::string out_pred_path;
	bool random_mode = false;
	uint64_t seed = 42;

	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--data" && i + 1 < argc) { data_path = argv[++i]; }
		else if (arg == "--pred" && i + 1 < argc) { pred_path = argv[++i]; }
		else if (arg == "--random") { random_mode = true; }
		else if (arg == "--seed" && i + 1 < argc) { seed = std::stoull(argv[++i]); }
		else if (arg == "--out" && i + 1 < argc) { out_pred_path = argv[++i]; }
		else if (arg == "-h" || arg == "--help") { printUsage(argv[0]); return 0; }
	}

	if (data_path.empty()) { printUsage(argv[0]); return 1; }

	try {
		auto items = loadGpqaCsv(data_path);
		if (items.empty()) {
			std::cerr << "No GPQA items loaded from " << data_path << "\n";
			return 2;
		}
		if (random_mode) {
			if (out_pred_path.empty()) out_pred_path = "preds_random.csv";
			writeRandomPredictions(items, out_pred_path, seed);
			std::cout << "Wrote random predictions to: " << out_pred_path << "\n";
			pred_path = out_pred_path;
		}
		if (pred_path.empty()) {
			printUsage(argv[0]);
			return 1;
		}
		auto pred = loadPredictionsCsv(pred_path);
		auto m = evaluate(items, pred);
		std::cout << "GPQA accuracy: " << m.accuracy * 100.0 << "% (" << m.correct << "/" << m.total << ")\n";
		return 0;
	} catch (const std::exception &ex) {
		std::cerr << "Error: " << ex.what() << "\n";
		return 3;
	}
} 
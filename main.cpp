
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdarg>
#include <vector>

struct Word
{
	std::array<char, 5> letters;
	int rep = 0;
};

struct Solution
{
	int r0;
	int r1;
	int r2;
	int r3;
	int r4;
};

struct Timer
{
	auto Seconds()
	{
		auto delta = std::chrono::high_resolution_clock::now() - start;
		return std::chrono::duration_cast<std::chrono::seconds>(delta).count();
	}

	auto Milliseconds()
	{
		auto delta = std::chrono::high_resolution_clock::now() - start;
		return std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
	}

	auto Microseconds()
	{
		auto delta = std::chrono::high_resolution_clock::now() - start;
		return std::chrono::duration_cast<std::chrono::microseconds>(delta).count();
	}

	std::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
};

static FILE* g_Log = nullptr;

void Log(const char* format, ...)
{
	char buffer[256];

	va_list args;
	va_start(args, format);
	vsprintf_s(buffer, format, args);
	va_end(args);

	printf(buffer);
	fprintf(g_Log, buffer);
}

bool IsLetter(char c)
{
	return 'a' <= c && c <= 'z';
}

std::vector<Word> Load(char const* filename)
{
	Timer timer;
	int size = 0;
	char* buffer = nullptr;

	if (auto f = fopen(filename, "rb"))
	{
		fseek(f, 0, SEEK_END);
		size = (int)ftell(f);
		fseek(f, 0, SEEK_SET);

		buffer = (char*)malloc(size + 1);
		buffer[0] = 0;

		fread(buffer, sizeof(char), size, f);
		buffer[size] = 0;

		fclose(f);
	}

	std::vector<Word> words;
	Word word;
	auto ptr = buffer;
	auto end = buffer + size;
	while (ptr < end)
	{
		// Pass whitespace, etc.
		while (ptr < end && !IsLetter(ptr[0]))
		{
			++ptr;
		}

		// Find next whole word
		auto ptr2 = ptr;
		while (ptr2 < end && IsLetter(ptr2[0]))
		{
			++ptr2;
		}

		// Store if it's the length we require
		if (ptr2 - ptr == word.letters.size())
		{
			for (int i = 0; i < word.letters.size(); ++i)
			{
				word.letters[i] = ptr[i];
			}
			words.push_back(word);
		}
		ptr = ptr2;
	}
	free(buffer);

	Log("Load [%s]: %zu (%lldms)\n", filename, words.size(), timer.Milliseconds());
	return words;
}

bool HasDoubleLetter(Word const& word)
{
	for (int i = 0; i < word.letters.size(); ++i)
	{
		for (int j = i + 1; j < word.letters.size(); ++j)
		{
			if (word.letters[i] == word.letters[j])
			{
				return true;
			}
		}
	}
	return false;
}

void RemoveDoubleLetters(std::vector<Word>& words)
{
	Timer timer;
	auto it = std::remove_if(words.begin(), words.end(), HasDoubleLetter);
	words.erase(it, words.end());

	Log("RemoveDoubleLetters: %zu (%lldus)\n", words.size(), timer.Microseconds());
}

std::vector<int> GenerateRepresentations(std::vector<Word>& words)
{
	Timer timer;

	std::vector<int> reps;
	for (auto& word : words)
	{
		int rep = 0;
		for (auto l : word.letters)
		{
			rep += 1 << (l - 'a');
		}
		word.rep = rep;
		if (reps.end() == std::find(reps.begin(), reps.end(), rep))
		{
			reps.push_back(rep);
		}
	}

	Log("GenerateRepresentations: %zu (%lldms)\n", reps.size(), timer.Milliseconds());
	return reps;
}

bool IsCommonLetter(int a, int b)
{
	return (a & b) != 0;
}

std::vector<Solution> FindSolutions(std::vector<int>& reps)
{
	struct Set
	{
		int r0;
		int r1;
		int r2;
		int r3;
		int r4;
		std::vector<int> options;
	};

	// Find all pairs of words that don't have any common letters
	std::vector<Set> pairs;
	{
		Timer timer;
		int count = 0;
		for (int i0 = 0; i0 < reps.size(); ++i0)
		{
			int r0 = reps[i0];
			Set set = { r0 };

			for (int i1 = i0 + 1; i1 < reps.size(); ++i1)
			{
				int r1 = reps[i1];

				if (!IsCommonLetter(r0, r1))
				{
					set.options.push_back(r1);
					++count;
				}
			}

			if (set.options.size() > 3)
			{
				pairs.push_back(set);
			}
		}
		Log("FindSolutions: %d pairs, %zu sets (%lldms)\n", count, pairs.size(), timer.Milliseconds());
	}

	// Pairs to triples
	std::vector<Set> triples;
	{
		Timer timer;
		int count = 0;
		for (int i0 = 0; i0 < pairs.size(); ++i0)
		{
			auto& pair = pairs[i0];
			auto& options = pair.options;

			for (int i1 = 0; i1 < options.size(); ++i1)
			{
				int r1 = options[i1];
				Set set = { pair.r0, r1 };

				for (int i2 = i1 + 1; i2 < options.size(); ++i2)
				{
					int r2 = options[i2];

					if (!IsCommonLetter(r1, r2))
					{
						set.options.push_back(r2);
						++count;
					}
				}

				if (set.options.size() > 3)
				{
					triples.push_back(set);
				}
			}
		}
		Log("FindSolutions: %d triples, %zu sets (%lldms)\n", count, triples.size(), timer.Milliseconds());
	}

	// Triples to quads
	std::vector<Set> quads;
	{
		Timer timer;
		int count = 0;
		for (int i0 = 0; i0 < triples.size(); ++i0)
		{
			auto& triple = triples[i0];
			auto& options = triple.options;

			for (int i1 = 0; i1 < options.size(); ++i1)
			{
				int r2 = options[i1];
				Set set = { triple.r0, triple.r1, r2 };

				for (int i2 = i1 + 1; i2 < options.size(); ++i2)
				{
					int r3 = options[i2];

					if (!IsCommonLetter(r2, r3))
					{
						set.options.push_back(r3);
						++count;
					}
				}

				if (set.options.size() > 2)
				{
					quads.push_back(set);
				}
			}
		}
		Log("FindSolutions: %d quads, %zu sets (%lldms)\n", count, quads.size(), timer.Milliseconds());
	}

	// Quads to solutions
	std::vector<Solution> out;
	{
		Timer timer;
		for (int i0 = 0; i0 < quads.size(); ++i0)
		{
			auto& quad = quads[i0];
			auto& options = quad.options;

			for (int i1 = 0; i1 < options.size(); ++i1)
			{
				int r3 = options[i1];
				Set set = { quad.r0, quad.r1, quad.r2, r3 };

				for (int i2 = i1 + 1; i2 < options.size(); ++i2)
				{
					int r4 = options[i2];

					if (!IsCommonLetter(r3, r4))
					{
						Solution s = { quad.r0, quad.r1, quad.r2, r3, r4 };
						out.push_back(s);
					}
				}
			}
		}
		Log("FindSolutions: %d solutions (%lldms)\n", out.size(), timer.Milliseconds());
	}

	return out;
}

void PrintWords(std::vector<Word> const& words, int rep)
{
	Log("{");
	for (auto word : words)
	{
		if (word.rep == rep)
		{
			Log(" %.*s", (int)word.letters.size(), &word.letters.front());
		}
	}
	Log(" }");
}

void PrintSolutions(std::vector<Solution> const& solutions, std::vector<Word> const& words)
{
	Timer timer;

	for (auto const& s : solutions)
	{
		Log("SOLUTION: ");
		PrintWords(words, s.r0);
		Log(" ");
		PrintWords(words, s.r1);
		Log(" ");
		PrintWords(words, s.r2);
		Log(" ");
		PrintWords(words, s.r3);
		Log(" ");
		PrintWords(words, s.r4);
		Log("\n");
	}

	Log("PrintSolutions: (%lldms)\n", timer.Milliseconds());
}

int main()
{
	// Open log
	g_Log = fopen("log.txt", "wb");
	Log("~~~ Welcome to WordFinder ~~~\n");

	// Start timer
	Timer timer;

	// Load word list
	//char const* filename = "words.txt";
	char const* filename = "words_alpha.txt";
	auto words = Load(filename);

	// Filter words with repeated letters
	RemoveDoubleLetters(words);
	auto reps = GenerateRepresentations(words);

	// Run the search
	auto solutions = FindSolutions(reps);
	PrintSolutions(solutions, words);

	// Record how long the process took
	Log("Total time: %lldms\n", timer.Milliseconds());
	fclose(g_Log);
}

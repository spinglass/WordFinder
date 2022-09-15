
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdarg>
#include <vector>

struct Word
{
	int rep = 0;
	std::array<char, 5> letters;
};

struct Entry
{
	int rep = 0;
	std::vector<Word> words;
};

struct Solution
{
	Entry e0;
	Entry e1;
	Entry e2;
	Entry e3;
	Entry e4;
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


void Print(Entry& en)
{
	Log("{");
	for (auto& w : en.words)
	{
		Log(" %.*s", (int)w.letters.size(), &w.letters.front());
	}
	Log(" }");
}

void Print(Solution& s)
{
	Log("SOLUTION: ");
	Print(s.e0);
	Log(" ");
	Print(s.e1);
	Log(" ");
	Print(s.e2);
	Log(" ");
	Print(s.e3);
	Log(" ");
	Print(s.e4);
	Log("\n");
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

void GenerateRepresentations(std::vector<Word>& words)
{
	Timer timer;

	for (auto& word : words)
	{
		int rep = 0;
		for (auto l : word.letters)
		{
			rep += 1 << (l - 'a');
		}
		word.rep = rep;
	}

	Log("GenerateRepresentations: (%lldus)\n", timer.Microseconds());
}

std::vector<Entry> FindAnagrams(std::vector<Word> in)
{
	Timer timer;
	std::vector<Entry> out;
	 
	for (auto word : in)
	{
		bool found = false;
		for (auto& en : out)
		{
			if (word.rep == en.rep)
			{
				found = true;
				en.words.push_back(word);
				break;
			}
		}
		
		if (!found)
		{
			Entry en;
			en.rep = word.rep;
			en.words = { word };
			out.push_back(en);
		}
	}

	Log("FindAnagrams: %zu (%lldms)\n", out.size(), timer.Milliseconds());
	return out;
}

bool IsCommonLetter(int a, int b)
{
	return (a & b) != 0;
}

std::vector<Solution> FindSolutions(std::vector<Entry>& in)
{
	struct Set
	{
		Entry* e0;
		Entry* e1;
		Entry* e2;
		Entry* e3;
		Entry* e4;
		std::vector<Entry*> options;
	};

	// Find all pairs of words that don't have any common letters
	std::vector<Set> pairs;
	{
		int count = 0;
		for (int i0 = 0; i0 < in.size(); ++i0)
		{
			auto e0 = &in[i0];
			Set set = { e0 };

			for (int i1 = i0 + 1; i1 < in.size(); ++i1)
			{
				auto e1 = &in[i1];

				if (!IsCommonLetter(e0->rep, e1->rep))
				{
					set.options.push_back(e1);
					++count;
				}
			}

			if (set.options.size() > 3)
			{
				pairs.push_back(set);
			}
		}
		Log("FindSolutions: %d pairs, %zu sets\n", count, pairs.size());
	}

	// Pairs to triples
	std::vector<Set> triples;
	{
		int count = 0;
		for (int i0 = 0; i0 < pairs.size(); ++i0)
		{
			if (i0 > 0 && i0 % 1000 == 0)
			{
				printf("%d / %zu : %d\n", i0, pairs.size(), count);
			}

			auto& pair = pairs[i0];
			auto& options = pair.options;

			for (int i1 = 0; i1 < options.size(); ++i1)
			{
				auto* e1 = options[i1];
				Set set = { pair.e0, e1 };

				for (int i2 = i1 + 1; i2 < options.size(); ++i2)
				{
					auto* e2 = options[i2];

					if (!IsCommonLetter(e1->rep, e2->rep))
					{
						set.options.push_back(e2);
						++count;
					}
				}

				if (set.options.size() > 3)
				{
					triples.push_back(set);
				}
			}
		}
		Log("FindSolutions: %d triples, %zu sets\n", count, triples.size());
	}

	// Triples to quads
	std::vector<Set> quads;
	{
		int count = 0;
		for (int i0 = 0; i0 < triples.size(); ++i0)
		{
			auto& triple = triples[i0];
			auto& options = triple.options;

			for (int i1 = 0; i1 < options.size(); ++i1)
			{
				auto* e2 = options[i1];
				Set set = { triple.e0, triple.e1, e2 };

				for (int i2 = i1 + 1; i2 < options.size(); ++i2)
				{
					auto* e3 = options[i2];

					if (!IsCommonLetter(e2->rep, e3->rep))
					{
						set.options.push_back(e3);
						++count;
					}
				}

				if (set.options.size() > 2)
				{
					quads.push_back(set);
				}
			}
		}
		Log("FindSolutions: %d quads, %zu sets\n", count, quads.size());
	}

	// Quads to solutions
	std::vector<Solution> out;
	{
		for (int i0 = 0; i0 < quads.size(); ++i0)
		{
			auto& quad = quads[i0];
			auto& options = quad.options;

			for (int i1 = 0; i1 < options.size(); ++i1)
			{
				auto* e3 = options[i1];
				Set set = { quad.e0, quad.e1, quad.e2, e3 };

				for (int i2 = i1 + 1; i2 < options.size(); ++i2)
				{
					auto* e4 = options[i2];

					if (!IsCommonLetter(e3->rep, e4->rep))
					{
						Solution s = { *quad.e0, *quad.e1, *quad.e2, *e3, *e4 };
						Print(s);
						out.push_back(s);
					}
				}

				if (set.options.size() > 2)
				{
					quads.push_back(set);
				}
			}
		}
		Log("FindSolutions: %d solutions\n", out.size());
	}

	return out;
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
	GenerateRepresentations(words);

	// Filter (and store) anagrams
	auto entries = FindAnagrams(words);

	// Run the search
	auto solutions = FindSolutions(entries);

	// Record how long the process took
	Log("Took: %llds\n", timer.Seconds());
	fclose(g_Log);
}

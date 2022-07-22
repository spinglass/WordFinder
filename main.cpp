
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdarg>
#include <vector>

using Word = std::array<char, 5>;

struct Entry
{
	Word base;
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
		Log(" %.*s", (int)w.size(), &w.front());
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
		if (ptr2 - ptr == word.size())
		{
			for (int i = 0; i < word.size(); ++i)
			{
				word[i] = ptr[i];
			}
			words.push_back(word);
		}
		ptr = ptr2;
	}
	free(buffer);

	return words;
}

bool IsAnagram(Word const& a, Word b)
{
	for (auto c : a)
	{
		bool found = false;
		for (auto& d : b)
		{
			if (c == d)
			{
				d = 0;
				found = true;
				break;
			}
		}
		if (!found)
		{
			return false;
		}
	}
	return true;
}

bool HasDoubleLetter(Word const& word)
{
	for (int i = 0; i < word.size(); ++i)
	{
		for (int j = i + 1; j < word.size(); ++j)
		{
			if (word[i] == word[j])
			{
				return true;
			}
		}
	}
	return false;
}

void RemoveDoubleLetters(std::vector<Word>& words)
{
	auto it = std::remove_if(words.begin(), words.end(), HasDoubleLetter);
	words.erase(it, words.end());

	Log("RemoveDoubleLetters: %zu\n", words.size());
}

std::vector<Entry> FindAnagrams(std::vector<Word> in)
{
	std::vector<Entry> out;
	 
	for (auto word : in)
	{
		bool found = false;
		for (auto& en : out)
		{
			if (IsAnagram(word, en.base))
			{
				found = true;
				en.words.push_back(word);
				break;
			}
		}
		
		if (!found)
		{
			Entry en;
			en.base = word;
			en.words = { word };
			out.push_back(en);
		}
	}

	Log("FindAnagrams: %zu\n", out.size());

	return out;
}

void SortBases(std::vector<Entry>& entries)
{
	for (auto& en : entries)
	{
		std::sort(en.base.begin(), en.base.end(), std::less<char>());
	}
}

bool IsCommonLetter(Word const& a, Word const& b)
{
	// Assumes a and b are sorted

	int i = 0;
	int j = 0;
	while (i < a.size() && j < b.size())
	{
		if (a[i] == b[j])
		{
			return true;
		}

		if (a[i] < b[j])
		{
			++i;
		}
		else
		{
			++j;
		}
	}
	return false;
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

				if (!IsCommonLetter(e0->base, e1->base))
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

					if (!IsCommonLetter(e1->base, e2->base))
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

					if (!IsCommonLetter(e2->base, e3->base))
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

					if (!IsCommonLetter(e3->base, e4->base))
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
	auto start = std::chrono::high_resolution_clock::now();

	// Load word list
	//char const* filename = "words.txt";
	char const* filename = "words_alpha.txt";
	auto words = Load(filename);
	Log("Load [%s]: %zu\n", filename, words.size());

	// Filter words with repeated letters
	RemoveDoubleLetters(words);

	// Filter (and store) anagrams
	auto entries = FindAnagrams(words);

	// Sort the base words to optimise the search
	SortBases(entries);

	// Run the search
	auto solutions = FindSolutions(entries);

	// Record how long the process took
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
	Log("Took: %llds\n", duration);
	fclose(g_Log);
}

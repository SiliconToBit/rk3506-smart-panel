#include "app/LyricParser.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <regex>

namespace fs = std::filesystem;

namespace
{
    std::string trimLeft(const std::string& input)
    {
        const size_t pos = input.find_first_not_of(" \t\r\n");
        if (pos == std::string::npos)
        {
            return "";
        }
        return input.substr(pos);
    }

    double parseFractionalSeconds(const std::string& fraction)
    {
        if (fraction.empty())
        {
            return 0.0;
        }

        if (fraction.size() == 1)
        {
            return static_cast<double>(fraction[0] - '0') / 10.0;
        }
        if (fraction.size() == 2)
        {
            return static_cast<double>((fraction[0] - '0') * 10 + (fraction[1] - '0')) / 100.0;
        }

        return static_cast<double>((fraction[0] - '0') * 100 + (fraction[1] - '0') * 10 + (fraction[2] - '0')) / 1000.0;
    }
} // namespace

namespace app
{
    bool LyricParser::loadFromFile(const std::string& lrcFilePath)
    {
        clear();

        std::ifstream ifs(lrcFilePath);
        if (!ifs.is_open())
        {
            return false;
        }

        const std::regex timeTag(R"(\[(\d{1,2}):(\d{1,2})(?:[\.:](\d{1,3}))?\])");
        std::string line;

        while (std::getline(ifs, line))
        {
            std::vector<double> timeMarks;
            size_t lastTagEnd = 0;

            for (std::sregex_iterator it(line.begin(), line.end(), timeTag), end; it != end; ++it)
            {
                const auto& match = *it;
                const int minutes = std::stoi(match[1].str());
                const int seconds = std::stoi(match[2].str());
                const std::string fraction = match[3].matched ? match[3].str() : "";
                const double timeSec = static_cast<double>(minutes) * 60.0 + static_cast<double>(seconds) +
                                       parseFractionalSeconds(fraction);

                timeMarks.push_back(timeSec);
                lastTagEnd = static_cast<size_t>(match.position() + match.length());
            }

            if (timeMarks.empty())
            {
                continue;
            }

            const std::string lyricText = trimLeft(line.substr(lastTagEnd));
            for (double timeSec : timeMarks)
            {
                m_lines.push_back({timeSec, lyricText});
            }
        }

        std::sort(m_lines.begin(), m_lines.end(),
                  [](const LyricLine& lhs, const LyricLine& rhs)
                  {
                      if (lhs.timeSec == rhs.timeSec)
                      {
                          return lhs.text < rhs.text;
                      }
                      return lhs.timeSec < rhs.timeSec;
                  });

        return !m_lines.empty();
    }

    bool LyricParser::loadForAudioFile(const std::string& audioFilePath)
    {
        fs::path lyricPath(audioFilePath);
        lyricPath.replace_extension(".lrc");
        return loadFromFile(lyricPath.string());
    }

    void LyricParser::clear()
    {
        m_lines.clear();
    }

    bool LyricParser::hasLyrics() const
    {
        return !m_lines.empty();
    }

    std::string LyricParser::getLyricAt(double positionSec) const
    {
        if (m_lines.empty())
        {
            return "";
        }

        if (positionSec < m_lines.front().timeSec)
        {
            return "";
        }

        auto it = std::upper_bound(m_lines.begin(), m_lines.end(), positionSec,
                                   [](double timeValue, const LyricLine& line) { return timeValue < line.timeSec; });

        if (it == m_lines.begin())
        {
            return "";
        }

        --it;
        return it->text;
    }
} // namespace app

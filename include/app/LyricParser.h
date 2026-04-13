#pragma once

#include <string>
#include <vector>

namespace app
{
    class LyricParser
    {
    public:
        struct LyricLine
        {
            double timeSec{0.0};
            std::string text;
        };

        bool loadFromFile(const std::string& lrcFilePath);
        bool loadForAudioFile(const std::string& audioFilePath);
        void clear();

        [[nodiscard]] bool hasLyrics() const;
        [[nodiscard]] std::string getLyricAt(double positionSec) const;

    private:
        std::vector<LyricLine> m_lines;
    };
} // namespace app

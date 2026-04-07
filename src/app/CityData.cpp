#include "app/CityData.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace app
{

    CityData& CityData::getInstance()
    {
        static CityData instance;
        return instance;
    }

    bool CityData::loadFromCSV(const std::string& path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            std::cerr << "[CityData] Failed to open file: " << path << '\n';
            return false;
        }

        m_csvPath = path;
        m_provinces.clear();

        std::string line;
        // 跳过第一行（标题）
        std::getline(file, line);

        // 跳过第二行（版本信息）
        std::getline(file, line);

        std::cout << "[CityData] Parsing CSV..." << std::endl;

        int lineNum = 2;
        while (std::getline(file, line))
        {
            lineNum++;
            std::stringstream ss(line);
            std::string cell;
            std::vector<std::string> cells;

            while (std::getline(ss, cell, ','))
            {
                cells.push_back(cell);
            }

            // CSV 格式: Location_ID,Location_Name_EN,Location_Name_ZH,...,Adm1_Name_ZH,Adm2_Name_ZH,...
            // 索引:     0,          1,              2,              ...,7,            9,...
            if (cells.size() < 10)
                continue;

            std::string locId = cells[0];    // Location_ID
            std::string locName = cells[2];  // Location_Name_ZH (区县名)
            std::string adm1Name = cells[7]; // Adm1_Name_ZH (省份)
            std::string adm2Name = cells[9]; // Adm2_Name_ZH (城市)

            // 只处理中国大陆城市（Location_ID 以 101 开头）
            if (locId.find("101") != 0)
                continue;

            // 查找或创建省份
            auto provIt = std::find_if(m_provinces.begin(), m_provinces.end(),
                                       [&adm1Name](const Province& p) { return p.name == adm1Name; });

            if (provIt == m_provinces.end())
            {
                Province prov;
                prov.name = adm1Name;
                m_provinces.push_back(prov);
                provIt = m_provinces.end() - 1;
            }

            // 查找或创建城市
            auto cityIt = std::find_if(provIt->cities.begin(), provIt->cities.end(),
                                       [&adm2Name](const City& c) { return c.name == adm2Name; });

            if (cityIt == provIt->cities.end())
            {
                City city;
                city.name = adm2Name;
                provIt->cities.push_back(city);
                cityIt = provIt->cities.end() - 1;
            }

            // 添加区县
            County county;
            county.name = locName;
            county.code = locId;
            cityIt->counties.push_back(county);
        }

        file.close();
        m_loaded = true;
        std::cout << "[CityData] Loaded " << m_provinces.size() << " provinces" << std::endl;

        return true;
    }

    bool CityData::isLoaded() const
    {
        return m_loaded;
    }

    std::string CityData::getProvinceList() const
    {
        std::string list;
        for (size_t i = 0; i < m_provinces.size(); ++i)
        {
            if (i > 0)
                list += "\n";
            list += m_provinces[i].name;
        }
        return list;
    }

    std::string CityData::getCityList(int provIndex) const
    {
        if (provIndex < 0 || provIndex >= static_cast<int>(m_provinces.size()))
            return "";

        const auto& cities = m_provinces[provIndex].cities;
        std::string list;
        for (size_t i = 0; i < cities.size(); ++i)
        {
            if (i > 0)
                list += "\n";
            list += cities[i].name;
        }
        return list;
    }

    std::string CityData::getCountyList(int provIndex, int cityIndex) const
    {
        if (provIndex < 0 || provIndex >= static_cast<int>(m_provinces.size()))
            return "";

        const auto& cities = m_provinces[provIndex].cities;
        if (cityIndex < 0 || cityIndex >= static_cast<int>(cities.size()))
            return "";

        const auto& counties = cities[cityIndex].counties;
        std::string list;
        for (size_t i = 0; i < counties.size(); ++i)
        {
            if (i > 0)
                list += "\n";
            list += counties[i].name;
        }
        return list;
    }

    int CityData::getProvinceCount() const
    {
        return static_cast<int>(m_provinces.size());
    }

    int CityData::getCityCount(int provIndex) const
    {
        if (provIndex < 0 || provIndex >= static_cast<int>(m_provinces.size()))
            return 0;
        return static_cast<int>(m_provinces[provIndex].cities.size());
    }

    int CityData::getCountyCount(int provIndex, int cityIndex) const
    {
        if (provIndex < 0 || provIndex >= static_cast<int>(m_provinces.size()))
            return 0;

        const auto& cities = m_provinces[provIndex].cities;
        if (cityIndex < 0 || cityIndex >= static_cast<int>(cities.size()))
            return 0;

        return static_cast<int>(cities[cityIndex].counties.size());
    }

    std::string CityData::getProvinceName(int provIndex) const
    {
        if (provIndex < 0 || provIndex >= static_cast<int>(m_provinces.size()))
            return "";
        return m_provinces[provIndex].name;
    }

    std::string CityData::getCityName(int provIndex, int cityIndex) const
    {
        if (provIndex < 0 || provIndex >= static_cast<int>(m_provinces.size()))
            return "";

        const auto& cities = m_provinces[provIndex].cities;
        if (cityIndex < 0 || cityIndex >= static_cast<int>(cities.size()))
            return "";

        return cities[cityIndex].name;
    }

    std::string CityData::getCountyName(int provIndex, int cityIndex, int countyIndex) const
    {
        if (provIndex < 0 || provIndex >= static_cast<int>(m_provinces.size()))
            return "";

        const auto& cities = m_provinces[provIndex].cities;
        if (cityIndex < 0 || cityIndex >= static_cast<int>(cities.size()))
            return "";

        const auto& counties = cities[cityIndex].counties;
        if (countyIndex < 0 || countyIndex >= static_cast<int>(counties.size()))
            return "";

        return counties[countyIndex].name;
    }

    std::string CityData::getFullCityName(int provIndex, int cityIndex, int countyIndex) const
    {
        std::string result;

        std::string prov = getProvinceName(provIndex);
        std::string city = getCityName(provIndex, cityIndex);
        std::string county = getCountyName(provIndex, cityIndex, countyIndex);

        if (!prov.empty())
            result += prov;
        if (!city.empty())
        {
            if (!result.empty())
                result += " ";
            result += city;
        }
        if (!county.empty())
        {
            if (!result.empty())
                result += " ";
            result += county;
        }

        return result;
    }

    std::string CityData::getCityCode(int provIndex, int cityIndex, int countyIndex) const
    {
        if (provIndex < 0 || provIndex >= static_cast<int>(m_provinces.size()))
            return "";

        const auto& cities = m_provinces[provIndex].cities;
        if (cityIndex < 0 || cityIndex >= static_cast<int>(cities.size()))
            return "";

        const auto& city = cities[cityIndex];

        // 如果指定了区县索引，返回区县代码
        if (countyIndex >= 0 && countyIndex < static_cast<int>(city.counties.size()))
        {
            return city.counties[countyIndex].code;
        }

        // 否则返回城市中心代码（第一个区县的代码）
        if (!city.counties.empty())
        {
            return city.counties[0].code;
        }

        return "";
    }

    std::string CityData::findCityCode(const std::string& province, const std::string& city,
                                       const std::string& county) const
    {
        // 查找省份
        auto provIt = std::find_if(m_provinces.begin(), m_provinces.end(), [&province](const Province& p)
                                   { return p.name.find(province) != std::string::npos; });

        if (provIt == m_provinces.end())
            return "";

        // 查找城市
        auto cityIt = std::find_if(provIt->cities.begin(), provIt->cities.end(),
                                   [&city](const City& c) { return c.name.find(city) != std::string::npos; });

        if (cityIt == provIt->cities.end())
            return "";

        // 如果指定了区县，查找区县
        if (!county.empty())
        {
            auto countyIt = std::find_if(cityIt->counties.begin(), cityIt->counties.end(), [&county](const County& c)
                                         { return c.name.find(county) != std::string::npos; });

            if (countyIt != cityIt->counties.end())
            {
                return countyIt->code;
            }
        }

        // 返回城市中心代码
        if (!cityIt->counties.empty())
        {
            return cityIt->counties[0].code;
        }

        return "";
    }

} // namespace app
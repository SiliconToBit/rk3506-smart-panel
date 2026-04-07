#pragma once

#include <string>
#include <vector>
#include <memory>

namespace app
{

    /**
     * @brief 区县数据
     */
    struct County
    {
        std::string name; ///< 区县名
        std::string code; ///< 和风天气城市代码
    };

    /**
     * @brief 城市数据
     */
    struct City
    {
        std::string name;             ///< 城市名
        std::string code;             ///< 城市代码（市级）
        std::vector<County> counties; ///< 下辖区县
    };

    /**
     * @brief 省份数据
     */
    struct Province
    {
        std::string name;         ///< 省份名
        std::vector<City> cities; ///< 下辖城市
    };

    /**
     * @brief 中国城市数据管理类
     *
     * 从 CSV 文件解析城市数据，提供省份/城市/区县查询
     *
     * CSV 格式（和风天气城市列表）：
     * Location_ID,Location_Name_ZH,Adm1_Name_ZH,Adm2_Name_ZH,...
     *
     * 使用示例：
     * @code
     * auto& cityData = CityData::getInstance();
     * cityData.loadFromCSV("/root/China-City-List-latest.csv");
     *
     * // 获取省份列表用于 Roller
     * std::string provList = cityData.getProvinceList();
     * lv_roller_set_options(roller, provList.c_str(), LV_ROLLER_MODE_NORMAL);
     *
     * // 获取选中省份的城市列表
     * std::string cityList = cityData.getCityList(provIndex);
     *
     * // 获取城市代码
     * std::string code = cityData.getCityCode(provIdx, cityIdx, countyIdx);
     * @endcode
     */
    class CityData
    {
    public:
        /**
         * @brief 获取单例实例
         */
        static CityData& getInstance();

        /**
         * @brief 从 CSV 文件加载城市数据
         * @param csvPath CSV 文件路径
         * @return 是否成功
         */
        bool loadFromCSV(const std::string& csvPath);

        /**
         * @brief 检查数据是否已加载
         */
        bool isLoaded() const;

        /**
         * @brief 获取省份列表（用于 LVGL Roller）
         * @return 换行分隔的省份名列表
         */
        std::string getProvinceList() const;

        /**
         * @brief 获取指定省份的城市列表（用于 LVGL Roller）
         * @param provIndex 省份索引
         * @return 换行分隔的城市名列表
         */
        std::string getCityList(int provIndex) const;

        /**
         * @brief 获取指定城市的区县列表（用于 LVGL Roller）
         * @param provIndex 省份索引
         * @param cityIndex 城市索引
         * @return 换行分隔的区县名列表
         */
        std::string getCountyList(int provIndex, int cityIndex) const;

        /**
         * @brief 获取省份数量
         */
        int getProvinceCount() const;

        /**
         * @brief 获取指定省份的城市数量
         */
        int getCityCount(int provIndex) const;

        /**
         * @brief 获取指定城市的区县数量
         */
        int getCountyCount(int provIndex, int cityIndex) const;

        /**
         * @brief 获取省份名称
         */
        std::string getProvinceName(int provIndex) const;

        /**
         * @brief 获取城市名称
         */
        std::string getCityName(int provIndex, int cityIndex) const;

        /**
         * @brief 获取区县名称
         */
        std::string getCountyName(int provIndex, int cityIndex, int countyIndex) const;

        /**
         * @brief 获取完整城市名称（省+市+区）
         */
        std::string getFullCityName(int provIndex, int cityIndex, int countyIndex = -1) const;

        /**
         * @brief 获取城市代码（用于天气 API）
         * @param provIndex 省份索引
         * @param cityIndex 城市索引
         * @param countyIndex 区县索引（-1 表示使用城市代码）
         * @return 和风天气城市代码
         */
        std::string getCityCode(int provIndex, int cityIndex, int countyIndex = -1) const;

        /**
         * @brief 根据名称查找城市代码
         * @param province 省份名
         * @param city 城市名
         * @param county 区县名（可选）
         * @return 城市代码，未找到返回空
         */
        std::string findCityCode(const std::string& province, const std::string& city,
                                 const std::string& county = "") const;

    private:
        CityData() = default;
        ~CityData() = default;

        // 禁止拷贝
        CityData(const CityData&) = delete;
        CityData& operator=(const CityData&) = delete;

        std::vector<Province> m_provinces;
        bool m_loaded = false;
        std::string m_csvPath;
    };

} // namespace app
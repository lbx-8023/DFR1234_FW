/**
 * @file    Display.cpp
 * @brief   OLED屏幕显示管理实现
 *
 * @details OLED屏幕显示管理实现
 */
#include "Display.h"
#include "ConfigParser.h"
#include "Wire.h"

/**
 * @brief 初始化屏幕显示
 *
 * @details 1. 初始化屏幕
 */
void ScreenDisplay::init()
{
    // 初始化U8g2库
    // u8g2 = U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C(U8G2_R0, OLED_SCL, OLED_SDA, U8X8_PIN_NONE);
    // (const u8g2_cb_t *rotation, uint8_t reset = U8X8_PIN_NONE, uint8_t clock = U8X8_PIN_NONE, uint8_t data = U8X8_PIN_NONE) : U8G2() {
    // Wire.setPins(L);
    u8g2=U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C(U8G2_R0,U8X8_PIN_NONE,OLED_SCL,OLED_SDA);
    // u8g2 = U8G2_SSD1306_128X32_UNIVISION_F_2ND_HW_I2C(U8G2_R0, U8X8_PIN_NONE);
    u8g2.begin();
    u8g2.clear();
    u8g2.enableUTF8Print();
    screenOn = true;

    lastActivityTime = millis(); // 记录初始化时间作为最后活动时间
}

/**
 * @brief 设置IP地址显示字符串
 *
 * @param str IP地址字符串（如"IP:192.168.1.100"）
 * @details 更新内部IP字符串，触发页面渲染时显示
 */
void ScreenDisplay::setIPStr(String str)
{
    ipStr = str;
}

/**
 * @brief 切换并显示指定页面
 *
 * @param page 页面索引（0：IP信息页；1：IO状态与I2C信息页）
 * @details 根据页面索引渲染不同内容，支持自动换行和滚动显示长文本
 */
void ScreenDisplay::showPage(int page)
{
    u8g2.clearBuffer();

    if (page == 0)
    {
        // 页面0：IP地址居中显示
        u8g2.setFont(u8g2_font_7x14B_tf);
        int textWidth = u8g2.getStrWidth(ipStr.c_str());
        int x = (128 - textWidth) / 2;
        int y = (32 - u8g2.getMaxCharHeight()) / 2 + u8g2.getAscent();
        u8g2.drawStr(x, y, ipStr.c_str());
    }
    else if (page == 1)
    {
        // 页面1：IO状态与I2C设备信息显示
        const int colCount = 4;
        const int spacing = 128 / colCount;

        u8g2.setFont(u8g2_font_unifont_t_symbols);
        for (int i = 0; i < colCount; ++i)
        {
            int triangleWidth = u8g2.getUTF8Width("△");
            int x = i * spacing + (spacing - triangleWidth) / 2;

            // 上排箭头闪烁逻辑
            // bool inBlink = (millis() - ioLabel[i].blinkUntil < 1000);
            bool inBlink = ioLabel[i].blinking;
            bool blinkState = (millis() / 300) % 2 == 0;
            if (inBlink)
            {
                u8g2.drawUTF8(x, 8, blinkState ? "▲" : "△"); // 闪烁：按状态显示实心或空心

                // u8g2.drawUTF8(x, 8, ioLabel[i].staus == 1 ? "▲" : "△"); // 闪烁：按状态显示实心或空心
                // if (ioLabel[i].staus == 1)
                // {
                //     ioLabel[i].staus = 0;
                // }
                // else
                // {
                //     ioLabel[i].staus = 1;
                // }
            }
            else
            {
                u8g2.drawUTF8(x, 8, "△"); // 闪烁结束后显示空心
            }

            // 下排箭头
            if (i < 2)
            {
                // bool inBlinkLow = (millis() - ioLabel[i + 4].blinkUntil < 1000);
                bool inBlinkLow = ioLabel[i+4].blinking;
                bool blinkStateLow = (millis() / 250) % 2 == 0;
                // LOG_INFO("PIN_%d  ,  blinking: %d, state: %d\n", i+4, inBlinkLow, blinkState);
                if (inBlinkLow)
                {
                    u8g2.drawUTF8(x, 33, blinkStateLow ? "▼" : "▽");
                    // u8g2.drawUTF8(x, 33, ioLabel[i + 4].staus == 1 ? "▼" : "▽");
                    // if (ioLabel[i].staus == 1)
                    // {
                    //     ioLabel[i].staus = 0;
                    // }
                    // else
                    // {
                    //     ioLabel[i].staus = 1;
                    // }
                }
                else
                {
                    u8g2.drawUTF8(x, 33, "▽");
                }
            }
        }

        // IO标签文本
        u8g2.setFont(u8g2_font_tinypixie2_tr);
        for (int i = 0; i < colCount; ++i)
        {
            int textWidth = u8g2.getStrWidth(ioLabel[i].text.c_str());
            int x = i * spacing + (spacing - textWidth) / 2;
            u8g2.drawStr(x, 12, ioLabel[i].text.c_str());

            if (i < 2)
            {
                u8g2.drawStr(x, 22, ioLabel[i + 4].text.c_str());
            }
        }

        // 显示 I2C 信息及滚动地址
        u8g2.setFont(u8g2_font_standardized3x5_tr);
        char tempStr[32];
        sprintf(tempStr, "I2C(%d):", i2cCNT);
        u8g2.setCursor(70, 24);
        u8g2.print(tempStr);

        int labelWidth = u8g2.getStrWidth(tempStr);
        int startX = 70 + labelWidth + 2;
        int clipWidth = u8g2.getDisplayWidth() - startX;

        scrollMessageWidth = u8g2.getStrWidth(scrollMessage.c_str());
        shouldScroll = scrollMessageWidth > clipWidth;//判断是否超出屏幕长度

        int currentX = shouldScroll ? (startX - scrollOffset) : startX;

        u8g2.setClipWindow(startX, 16, startX + clipWidth, 31);
        u8g2.drawStr(currentX, 24, scrollMessage.c_str());
        u8g2.setMaxClipWindow();
    }

    u8g2.sendBuffer();
}

/**
 * @brief 主循环更新函数
 *
 * @details 处理按钮事件、屏幕滚动、超时休眠及IO状态更新
 */
void ScreenDisplay::loop()
{

    unsigned long now = millis();
    // 处理 IO 闪烁超时
    for (int i = 0; i < 6; ++i)
    {
        if (ioLabel[i].blinking && now > ioLabel[i].blinkUntil)
        {
            ioLabel[i].blinking = false;
        }
    }

    if (buttonPressed)
    {
        buttonPressed = false;

        if (!screenOn)
        {
            screenOn = true;
            currentPage = 0;
            showPage(currentPage);
        }
        else
        {
            currentPage = (currentPage + 1) % totalPages;
            showPage(currentPage);
        }

        lastActivityTime = millis();
    }

    // 处理滚动逻辑（仅屏幕开启时生效）
    if (screenOn && (millis() - lastScrollTime >= scrollInterval))
    {
        scrollOffset += 1;
        if (scrollOffset > scrollMessageWidth)
            scrollOffset = 0;

        showPage(currentPage);
        lastScrollTime = millis();
    }

    // 超时自动关闭屏幕
    if (screenOn && (millis() - lastActivityTime > screenTimeout))
    {
        u8g2.clearDisplay();
        screenOn = false;
    }
}

/**
 * @brief 从配置键值对更新IO标签
 *
 * @param kv_pairs 配置键值对数组
 * @param max_entries 键值对数量
 * @details 根据配置文件中的"IOx"键值更新IO标签文本（如"DHT11"、"INPUT"）
 */
void ScreenDisplay::updateHeaderFromKeyValue(KeyValue *kv_pairs, int max_entries)
{
    const char *validLabels[] = {
        "DHT11", "DS18B20", "OUTPUT", "SERVO180", "SERVO360", "WS2812", "INPUT", "HEARTRATE", "EMG", "SERVO300"};

    char tempKey[8];
    for (uint8_t i = 0; i < IO_PORT_NUM; i++)
    {
        // sprintf(tempKey, "IO%d", i + 1);
        sprintf(tempKey, "P%d", i + 1);
        const char *val = get_value_case_insensitive(kv_pairs, max_entries, tempKey);

        bool matched = false;
        // 匹配有效标签（忽略大小写）
        if (val != NULL)
        {
            for (const char *valid : validLabels)
            {
                if (strcasecmp(val, valid) == 0) // 忽略大小写比较
                {
                    ioLabel[i].text = String(valid); // 保留规范大小写
                    matched = true;
                    break;
                }
            }
        }
        // 未匹配则设为"N/A"
        if (!matched)
        {
            ioLabel[i].text = "N/A";
        }
    }
}

/**
 * @brief 设置I2C设备信息
 *
 * @param cnt I2C设备数量
 * @param str I2C地址字符串（如"0X50,0X51"）
 * @details 更新I2C设备数量和地址字符串，触发滚动显示
 */
void ScreenDisplay::setI2cStr(uint8_t cnt, String str)
{
    i2cCNT = cnt;
    scrollMessage = str;
}

/**
 * @brief 更新IO状态
 *
 * @param ioCNT IO端口索引（0-based）
 * @param state 端口状态（true:激活，false:关闭）
 * @details 限制端口索引范围，更新对应标签的状态值
 */
void ScreenDisplay::setStaus(uint8_t ioCNT, bool state)
{
    if (ioCNT < 6)
    {
        ioLabel[ioCNT].staus = state;
        ioLabel[ioCNT].blinking = true;
        ioLabel[ioCNT].blinkUntil = millis() + 1000; // 设置闪烁持续时间为1秒
    }
}
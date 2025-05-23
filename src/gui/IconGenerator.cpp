#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>

// 创建简单的BMP图像文件（32x32，24位色）
void createSimpleBMP(const std::string &filename, uint8_t r, uint8_t g, uint8_t b)
{
    const int width = 32;
    const int height = 32;
    const int bpp = 24; // 24位色

    // BMP文件头 (14字节)
    struct BMPHeader
    {
        uint16_t signature;  // 'BM'
        uint32_t fileSize;   // 文件大小
        uint16_t reserved1;  // 保留
        uint16_t reserved2;  // 保留
        uint32_t dataOffset; // 数据偏移
    } header;

    // BMP信息头 (40字节)
    struct BMPInfoHeader
    {
        uint32_t size;            // 信息头大小
        int32_t width;            // 图像宽度
        int32_t height;           // 图像高度
        uint16_t planes;          // 色彩平面数
        uint16_t bitsPerPixel;    // 每像素位数
        uint32_t compression;     // 压缩类型
        uint32_t imageSize;       // 图像大小
        int32_t xPixelsPerMeter;  // 水平分辨率
        int32_t yPixelsPerMeter;  // 垂直分辨率
        uint32_t colorsUsed;      // 使用的颜色数
        uint32_t colorsImportant; // 重要的颜色数
    } infoHeader;

    // 计算每行的字节数（需要是4的倍数）
    int rowSize = ((width * bpp + 31) / 32) * 4;
    int dataSize = rowSize * height;

    // 填充文件头
    header.signature = 0x4D42; // "BM"
    header.fileSize = 14 + 40 + dataSize;
    header.reserved1 = 0;
    header.reserved2 = 0;
    header.dataOffset = 14 + 40;

    // 填充信息头
    infoHeader.size = 40;
    infoHeader.width = width;
    infoHeader.height = height;
    infoHeader.planes = 1;
    infoHeader.bitsPerPixel = bpp;
    infoHeader.compression = 0;
    infoHeader.imageSize = dataSize;
    infoHeader.xPixelsPerMeter = 2834; // 72dpi
    infoHeader.yPixelsPerMeter = 2834; // 72dpi
    infoHeader.colorsUsed = 0;
    infoHeader.colorsImportant = 0;

    // 生成像素数据（BGR顺序）
    std::vector<uint8_t> pixelData(dataSize, 0);
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = y * rowSize + x * 3;
            if (x > 4 && x < width - 4 && y > 4 && y < height - 4)
            {
                pixelData[index] = b;     // B
                pixelData[index + 1] = g; // G
                pixelData[index + 2] = r; // R
            }
            else
            {
                // 图像边缘使用暗色边框
                pixelData[index] = b / 3;     // B
                pixelData[index + 1] = g / 3; // G
                pixelData[index + 2] = r / 3; // R
            }
        }
    }

    // 写入文件
    std::ofstream file(filename, std::ios::binary);
    if (file)
    {
        file.write(reinterpret_cast<char *>(&header), sizeof(header));
        file.write(reinterpret_cast<char *>(&infoHeader), sizeof(infoHeader));
        file.write(reinterpret_cast<char *>(pixelData.data()), dataSize);
        file.close();
        std::cout << "Created " << filename << std::endl;
    }
    else
    {
        std::cerr << "Failed to create " << filename << std::endl;
    }
}

/* // Comment out the main function to avoid multiple definitions
int main(int argc, char **argv)
{
    // 创建图标目录
    std::string baseDir = "d:/CodeMaster/GitHub/Warehouse-sch/homework/GUI/resources/icons/";

    // 创建四种设备图标
    createSimpleBMP(baseDir + "storage_in.bmp", 50, 200, 50);  // 入库接口 - 绿色
    createSimpleBMP(baseDir + "storage_out.bmp", 200, 50, 50); // 出库接口 - 红色
    createSimpleBMP(baseDir + "work_in.bmp", 50, 50, 200);     // 入库作业口 - 蓝色
    createSimpleBMP(baseDir + "work_out.bmp", 200, 200, 50);   // 出库作业口 - 黄色

    return 0;
}
*/

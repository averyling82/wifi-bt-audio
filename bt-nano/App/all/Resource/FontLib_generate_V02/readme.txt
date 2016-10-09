使用方法：
    1. 将需要替换的字体文件放在根目录下(Fron12.bin和Fron16.bin)。
    2. 将需要提取的字库文件以特定形式保存为BMP文件，并存入对应的文件夹fron16、fron12 
       a. fron12 格式要求：16* 12、单色模式、并以Unicode编码命名文件。
       b. fron12 格式要求：16* 16、单色模式、并以Unicode编码命名文件。
    3. 执行fontlib.exe，将重新生成Fron12.bin和Fron16.bin。
    4. 将\Development\firmware_generate_eMMC中的字库文件替换
    5. 重新生成并下载


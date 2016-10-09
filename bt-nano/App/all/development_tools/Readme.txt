1. Nand Flash 使用“firmware_generate”     生成固件
2. eMMC Flash 使用“firmware_generate_eMMC”生成固件
3. SPI Flash  使用“firmware_generate_SPI” 生成固件
4. 统一       使用“firmware_upgrade”      下载固件

5. SPI固件说明：
  由于SPI Flash容量比较小，固件生成工具仅将代码打包；
  资源文件“..\firmware_generate_SPI\Resource”需要
  拷贝到磁盘根目录下才能使用，用户修改的资源文件需要
  按照相同目录下的文件名称命名。
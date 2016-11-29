RKNanoD  Wireless Audio SDK V1.0
.....

RKNanoD  Wireless Audio SDK V1.1
.....

RKNanoD  Wireless Audio SDK V1.2
.....

RKNanoD  Wireless Audio SDK V1.3更新如下：

1，OTA升级
   说明：如果你的SPI BOOT的板子曾经不小心烧写过EMMC BOOT固件，请先使用如下命令，在使用此SDK
   connect lun
   lun.test

   烧写过EMMC BOOT固件的SPI BOOT的板子，OTA起不来。
   如果我们使用开发工具或者量产工具烧写的固件大小记为X
   OTA升级的固件的大小记为Y，则必须满足如下条件,Y>=X, && Y-X < 1.5M(1M = 1024 * 1024 BYTES)
   
   connect fw
   fw.inf
   以上命令可以查看固件信息
   
	[000547.36]product sn: 
	[000547.36]Ver: 00.00.0000
	[000547.36]Date:2016-05-19
	[000547.36]Firmave Size = 4608
	[000547.36]Lun 0 Size = 6144
	[000547.37]cur fw is fw1

  当前固件为FW1时，方可进行OTA升级

  fw.update更新固件，升级完之后系统会重启，开机会慢，请等待，起来之后，使用fw.inf如果当前固件为fw2表示升级成功
  否则升级失败。
  
  生成的固件不能大于LUN 0的SIZE,否则系统起不来， LUN0的大小根据物理大小可以分配，必须保证LUN1大小为1M


2，MSC切换MASKROM升级

3，修改音频播放器BUG及其AAC歌曲的支持, 关闭_FADE_PROCESS_系统可以正常工作

4，FM功能实现

5，音频设备链路，支持全双工

6，文件系统支持删除，修正部分BUG，异常操作，文件系统保护

7，开放B核调试平台
   KEIL 工程下有2个工程RkNanoD_Wireless_Audio A核工程，B_CORE B核工程， 调试方式如下：
   当_SUPPORT_BB_SYSTEM_关闭时，整个系统不使用B核
   当_SUPPORT_BB_SYSTEM_打开时，BB_SYS_JTAG打开，B核可以使用JTAG1,方法如下，第一次打开_SUPPORT_BB_SYSTEM_， BB_SYS_JTAG时，A核需要重新编译，然后烧录，选择你要音乐，此时A核会被挂住，等待B核的反馈，B核的代码可以通过JTAG1下载，调试。
   当_SUPPORT_BB_SYSTEM_打开时，BB_SYS_JTAG关闭时，B核不能使用JTAG,当B核修改时，必须编译A核烧录。

8，开放基于B核调试平台的编解码平台
   MAIN2里面有完整的命令交互，第三方编解码只需要通过pCODECS.c(A核)， pCODECS2.c(B核)接入即可。
   工程的创建请参考B_CORE
   
   请在创建工程中的KEIL设置选项卡，C/C++选项下的Processer Symbols栏的define项，填填入XXX_DEC_INCLUDE2, B_CORE_DECODE
   其中XXX_DEC_INCLUDE2为按照我们编解码框架定义的第三方宏

9, 新增带序列号功能的量产工具

10，系统频率更改, 增加系统API

11, 修改开关机保存的BUG

12，更改录音控制流程

13，支持Line in

14, 解决部分语言设置的错误

15，USB提速及其BUG的修改

16，SDMMC驱动的修改

17，DMA驱动修改，解决通道泄漏

18，FIFO驱动的修改，增加HTTP PLAY的DEMO，增加API写入用来通知FIFO写入出错，防止FIFO死锁。

19，修改GUI BUG, 状态栏显示抖动。

20，修改DLNA快速切换死机

21, 丰富系统调试方式

22, 加大TCP窗口

23, 解决无屏关机死机问题


RKNanoD  Wireless Audio SDK V1.5更新如下：

1,解决V1.3版本中存在并已经发现的BUG, BUG来自客户，自测，内测等。

2，新增多声道 SDK工程

3，新增对WIFI,BT模组的支持

4，新增AMR放音和录音

5，新增BT接电话

6，新增媒体库
   媒体库同时支持多盘。

7，新增系统待机机制，目前支持I,II级待机，II级待机完成电流为1.45MA

   当系统没有人为事件改变状态超过一定时间，进入I级待机状态，I待机关闭GUI系统

   当进入I级待机超过一定时间，自动进入II级待机，II级待机是一个持续的过程，会按照设备和线程既定的待机模式，将其待机。

   人为事件包括，按键，A2DP请求连接，BT CALL请求连接，DLNA请求连接等，人为事件发生时，系统被唤醒。

   如果人为事件的通道被休眠，将无法唤醒系统，只能通过PLAY ON按键或者其他第三方唤醒机制（比如RTC）唤醒。
   
   WIFI待机事件是LWIP线程长时间无动作，如果防止WIFI待机，请保持心跳时间小于LWIP线程待机时间。


8，新增设备，线程待机机制，只有当所有的设备和线程都处于I级待机时，系统才算完成II级待机。 
   
   设备待机模式分为3种模式：
   
   拒绝模式：不按待机机制的设备，默认为拒绝模式，按照待机机制，模式设置为DISABLE_MODE的设备。
   
   使能模式：该设备在没有使用超过一定时间，就会进入待机， 参考DISPLAYDEV
  
   强行模式：该设备在不工作超过一定时间，就会进入待机，参考LCD DEV

   线程待机模式只有一种，请参考SHELLTASK
   

9，新增多盘（隐藏盘）机制。
   系统开机会加载3个即A盘（可通过_HIDDEN_DISK1_关闭）,B盘（可通过_HIDDEN_DISK2_关闭）,C盘，对应盘符分别为RKOSA,RKOSB,RKOSC
   PC无法更改，通过修改源码可更改，如果C盘存在钥匙 RKOSA.KEY, RKOSB.KEY可打开对应的盘，钥匙可通过修改源码更改。
   SPI BOOT的资源存放顺序分别是A, B, C三个盘，即如果有A盘，存A盘，如果没有A盘，有B盘，存B盘，否则存C盘。


RKNanoD  Wireless Audio SDK V1.6更新如下：
 
 1,支持SOFT AP，WLAN->WIFI POWER里面有2个启动选项，一个是ON STA, 另一个ON AP, 从AP启动后，SSID为rkos-iot, 	password:123123123。

 2, DLNA支持全格式和SEEK功能。

 3，修改无屏操作逻辑，使其更加完善。

 4，增加看门口。

 5，增加大量宏控制，方便代码分割。

 6，解决系统无法从待机模式2.8进入待机模式2.9。

 7，优化WLAN启动，连接速度。

 8，解决V1.5版本中存在并已经发现的BUG, BUG来自客户，自测，内测等。

 
   

  

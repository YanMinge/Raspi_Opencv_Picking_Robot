# Raspi_Opencv_Picking_Robot
This is code for a picking robot

【软硬件需求】
1. 树莓派2开发板一块
2. Makeblock机械零件若干，详细可见帖子(已包含电机，舵机等执行器件):
   http://bbs.makeblock.cc/thread-716-1-1.html
3. opencv 2.4.10  (使用其它版本可能会遇到兼容性问题)
4. Makeblock Orion主板一块
5. USB摄像头(尽量买linux免驱的摄像头，避免额外工作)
6. 充电宝一个(提供5V/2A输出给树莓派供电)

【Opencv的环境搭建】
    我使用的opencv版本是 2.4.10, 在树莓派2上搭建环境的方法请参考的是这个网页的说明
    http://www.jb51.net/article/63103.htm
    不习惯使用虚拟环境的同学可以自行在树莓派上直接搭建Opencv的开发环境，推荐用建立虚拟环境的方法，这样可以避免多个python版本同时共存时出现冲突。
   
【其他需要使用到的库】
    除了opencv环境，设计中还需要使用到串口，因此 wiringpi，serial都是必须安装的。
    如果选择虚拟环境安装的，请先进入虚拟环境再安装依赖库 ----- “workon XXXX“
    (1) 安装 wiringpi
        sudo pip install -y wiringpi
   （2）安装 serial
        sudo pip install -y pySeria
        
【程序说明】
    1. Orion中的程序是Orion板子的程序，Orion开发板是MakeBlock基于arduino UNO硬件开发的一块集成有蜂鸣器，电机驱动的主板。连线简易，可以快速打造一些DIY的作品。
       Orion程序主要用来接收树莓派探测到的目标位置更新，然后根据这个目标位置，来控制执行机构的动作（电机和舵机）.主控程序中设置了一个状态机来进行捡球这个动作
       的分解，包括 找球(FIND_THE_BALL), 对准球的位置(WALKED_TURN_TO_BALL), 行驶到目标点(WALKED_AROUND_BALL), 捡球(PICKED_UP_BALL), 捡球结束(PICKED_UP_FINISH),
       还有一个状态是超时检测，避免机器人进入控制死区。
2. Raspi的程序主要完成对球的检测，利用opencv的 inRange 函数对球的颜色进行过滤，滤除环境中的其它干扰。然后利用opencv的基本形态转换erode(腐蚀)，dilate（膨胀)
       进行二次处理，如果摄像头视角内没有其它接近球体颜色的干扰，这个滤波效果基本就已经可以做到目标检索了。
       在代码设计中，我最初的设想是利用球体颜色做第一次过滤处理，然后再利用球体形状做第二次过滤处理，这样可以更大程度的避免外界环境的干扰。但实际测试中，网球
       经常不能被识别为圆形(估计是受网球纹路和商标花纹的影响，所以我只能忍痛注释了这段代码)。
       少了这段代码，外界如果有和球体颜色接近的目标，就会极大干扰Picking Robot的工作。如果你是用的纯色的圆球，加上这段代码，应该可以适应更加复杂的外界环境。
  
        
    
    
   
   

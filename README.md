# OS
操作系统


## 多线程信号量的使用

#### kk.c

使用信号量实现了多个生产者，多个消费者，多个缓冲区的消费者和生产者的解决方案。

```

模拟场景： 15个生产者，10个消费者，5个缓冲区，缓冲区数据结构包含数据、生产线程tid、写入时间。

编译时使用： 

gcc -o kk kk.c -lpthread
```
## 基于内存映射的设备驱动程序

通过添加内核模块实现一个基于内存映射的杂项设备驱动程序。


```

拓展：

该模块只实现了显示内存映射区域信息的功能，而且该信息是固定；

拓展部分实现了将当前进程在内存映射后的vma区域的信息显示出来。

编译时：


$make

$sudo insmod miscdev_map.ko

$sudo chmod a+rw /dev/mymap

$gcc -o miscdev_maptest miscdev_maptest.c

$./miscdev_maptest

```



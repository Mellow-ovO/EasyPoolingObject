# 简单对象池
- 支持蓝图、C++
- 支持设置对象池最大数量
- 支持对象池满后异步等待获取对象
- 支持设置异步等待优先级，支持设置队列上限
- 封装为 UK2Node，保持了类似于 Construct Object 的快速设置 ExposeOnSpawn 变量的引脚

## 快速使用
1. 继承接口
   <p align = "center">
   <img src = "Resources/1.png" height = 300>
   </p>

2. 重载相关函数
   <p align = "center">
   <img src = "Resources/2.png" height = 200>
   </p>

3. 通过接口获取对象
   <p align = "center">
   <img src = "Resources/4.png" height = 600>
   </p>

## 其他
- 设置对象池数量上限和等待队列上限
   <p align = "center">
   <img src = "Resources/3.png" height = 200
   >
   </p>

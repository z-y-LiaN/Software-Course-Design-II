---
title: 【SE】词法分析器和语法分析器的设计
date: 2022-04-15 22:45:34
copyright: false
tags: [编译原理]
categories: 专业课
mathjax: true

---

- 主要内容
  - 词法分析器的原理和设计
  - 语法分析器的原理和设计

---



<!--more-->

# 1. 词法分析器

## 1.1 基本原理

- **正规文法**

  - 也称**3型**文法 G=(VN,VT,S,P)
  - 其中P中的每一条规则都有下述形式：`A->aB或A->a`，其中A和B是非终结符，a是终结符的闭包

- **不确定的有穷自动机NFA**

  > 一个不确定的 有穷自动机M 是一个五元组：$M=（K,\sum,f,S,Z)$

  - K：有穷集，每个元素称为一个**状态**
  - Σ：有穷字母表，每一个元素称为一个**输入符号**

  - f：一个映射
  - S ⊆ K：非空初态集（对于**DFA，初态唯一**）
  - Z ⊆ K：终态集

- **字符串t能被DFA所接收**

  - 对于$\sum^{*}$中的任何符号串t，若存在一条从初态节点到某一终态节点的道路，且这条道路的所有弧的标记符连接成的符号等于t，则称t可被这个DFA所接收（识别），若DFA的初态节点同时又是终态节点，则空字($\epsilon$)可为DFA所接收


---

## 1.2 设计思路

​	最先开始先以课本上给出的简单构词规则进行编写代码，进行调试，然后逐步扩充和完善构词规则，进行编码和调试。严格按照词法分析流程，根据规定的正规文法，先**构建NFA**，再利用子集法**确定为DFA**，分模块逐步完成主要函数。最后再通过DFA，读入测试程序进行扫描进行词法分析，得到**输出的token序列**或者错误信息。生成 token 列表（三元组：所在行号，类别，token 内容）。由于算法所用数据结构和C++中许多已有数据结构概念相符，因此可考虑用C++代码编写，充分利用C++中的STL库，十分方便算法的实现。

## 1.3 文件结构

| 路径                        | 文件说明                                               |
| --------------------------- | ------------------------------------------------------ |
| Lexical\TXT\GRAMMAR.txt     | 存放构词规则（正规文法）                               |
| Lexical\TXT\INCHAR.txt      | 存放终结符                                             |
| Lexical\TXT\KEYWORDS.txt    | 存放关键字                                             |
| Lexical\TXT\LIMITER.txt     | 存放界符(限定符）                                      |
| Lexical\TXT\OPERATOR.txt    | 存放操作符                                             |
| Lexical\TXT\ROW.txt         | 存放源程序中每一行的token数，便于语法分析器的报错提示  |
| Lexical\TXT\SOURCE.txt      | 存放源程序代码                                         |
| Lexical\TXT\TOKEN_TRIAD.txt | 存放输出的token序列（三元组）                          |
| Lexical\TXT\TOKEN.txt       | 存放输出的token序列（二元组）                          |
| Lexical\TXT\TOKEN.txt       | 存放输出的token序列（二元组）                          |
| Lexical\TXT\WRONG.txt       | 存放语法分析器的报错信息，同时便于语法分析器的分析判断 |
| Lexical\lexical.cpp         | 函数实现                                               |
| Lexical\path.h              | 文件路径定义的头文件                                   |
| Lexical\lexical.h           | 函数和变量声明的头文件                                 |
| Lexical\main.cpp            | 主函数                                                 |

## 1.4 算法流程

​	根据课程要求中提到的词法分析程序的推荐处理逻辑完成词法分析：根据设定的正规文法，生成 NFA，再将NFA确定化生成 DFA， 根据 DFA 编写识别 token 的程序，从头到尾从左至右识别输入的源代码，生成 token列表（三元组：所在行号，类别，token 内容）。总体流程图如下：

<img src="https://i.bmp.ovh/imgs/2022/05/03/b31be7f16a2e2127.png" style="zoom: 40%;" />

## 1.5 构词规则

基于本课程考核要求的内容，确定词法分析的构词如下：

<img src="https://i.bmp.ovh/imgs/2022/05/03/a19bf29f8f13c3e2.png" style="zoom:40%;" />

因此，根据构词规则设置正规文法，文法中的各项产生式的作用和之间的**逻辑关系和分类**如下：

1. 界符(限定符)

<img src="../blogImages/README/image-20220503095219277.png" alt="image-20220503095219277" style="zoom: 30%;" />

2. 标识符

   <img src="../blogImages/README/image-20220503095318581.png" alt="image-20220503095318581" style="zoom: 33%;" />

3. 运算符

   <img src="../blogImages/README/image-20220503095435881.png" alt="image-20220503095435881" style="zoom: 33%;" />

4. 常量

   <img src="../blogImages/README/image-20220503095607966.png" alt="image-20220503095607966" style="zoom:50%;" />

## 1.6 具体实现

### （1）初始化所有加载项：void init()

预处理，从各文本文件中加载终结符、关键字、操作符、界符到各自的数据结构中。

| 类型   | 数据结构                | 函数名                |
| ------ | ----------------------- | --------------------- |
| 终结符 | vector<char>INCHAR      | void load_inchar();   |
| 关键字 | vector<string> KEYWORDS | void load_keywords(); |
| 操作符 | vector<string> OPT      | void load_opt();      |
| 界符   | vector<string>LIMITER   | void load_limiter();  |

### （2）创建NFA：void createNFA()

- **数据结构定义**

  - NFA中的一条边的数据结构，结构体存储三元组的形式：边的起点，边的输入（权重），边的终点

    <img src="../blogImages/README/image-20220503100210138.png" alt="image-20220503100210138" style="zoom: 50%;" />

    ```cpp
    typedef struct Triad {
      char startPoint, input, endPoint;
    } Triad;
    ```

  - NFA的数据结构： 结构体保存`initialState`-初态，`finalState`-终态，`f`-NFA中所有边的集合

    ```cpp
    typedef struct NFA {
      char initialState;
      vector<char> finalState;
      vector<Triad> f;
    } NFA;
    ```

- **读入的正规文法创建NFA**

  - 根据**右线性**正规文法的转换规则（注：t 为 VT 或ε）：
    	① 增加一个终态结点，开始符号对应的结点作为初态
    	② 对形如 A→t 的规则，引一条从A到终态结点的弧，标记为t
    	③ 对形如 A→tB 的规则，引一条从A到B的弧，标记为t

  - 算法具体流程图如下，由于NFA的终态集合采用的是vector，最后需要对vector的元素进行去重，但代码运行时即使不去重也没有影响。

    <img src="../blogImages/README/image-20220503100618138.png" alt="image-20220503100618138" style="zoom:50%;" />

### （3）将NFA转换成DFA：void NFA_TO_DFA()

​	首先，定义DFA的数据结构，利用结构体保存初态-initialState、终态集合-finalState、边的集合-f；在定义DFA的边的集合时，并没有采用像NFA一样的结构体来定义边，而是采用一个二维数组`f[MAX_NODES][MAX_NODES]`，比如`f[state][input]=nextState`就表示从当前状态`state`输入字符`input`后转移到下一个状态`nextState`，将char隐式地转换成int类型作为数组下标，在代码中利用change函数实现，比如初始状态表示字符`S`对应int中0，其余状态`A`对应int中的1，`B`对应int中的2……以此类推。的实现了O(1)查找，提高了算法的效率。

```cpp
typedef struct DFA {
  char initialState;
  vector<char> finalState;
  int f[MAX_NODES][MAX_NODES];  
} DFA;
```

算法的关键部分的伪代码如下：

```cpp
开始e-closure(K0)令C为中唯一成员，并且它是未被标记的。
While (C 中存在尚未标记的子集T) do
{    标记T；
     For(每个输入字符a) do
      {   U:=e-closure(move(T,a))
          If (U不在C中) then
          { 将U作为未标记的子集加载C中；}
}
```

其中，需要求闭包和move集：

- **求状态集T的闭包:**

  - 对应代码的函数名为`set<char> e_closure(set<char> T)`。

  - 传入参数，类型为`set<char> `的状态集T，函数返回状态集T的一个闭包。
  - 求T的闭包即是求状态集**T中的任何状态**，及**经过任意条ε弧所能到达的状态**的集合。
  - 由于集合的概念和C++的STL中的set概念一致，所以采用的是set类型来保存所有满足要求的状态，这样就没有重复元素，不需要去重。

- **求集合I的move集：**

  - 对应代码的函数名为`set<char> move(set<char> I, char input)`。

  - 传入参数，类型为`set<char>`  的集合I，和一个char类型的字符input，求它的move集，即是求所有可以从I中的某一状态经一条input弧所能到达的状态，返回满足要求的状态的集合。

定义完求闭包和求move集的函数后，编写NFA转换成DFA的算法。根据伪代码转换的具体算法流程图如下：

<img src="../blogImages/README/image-20220503111033602.png" alt="image-20220503111033602" style="zoom:50%;" />

### （4）读入源程序进行词法分析：void scanSourceCode()

​	首先，在读入源程序之前，需要将源程序一些不必要的换行和tab符号过滤掉，然后存到数组中，然后对这些已经用空格分隔该来的预处理字符串序列进行处理，将一个个字符串分割成一个个单词，初始化对应的单词类型为空串，具体算法执行流程如下：

<img src="../blogImages/README/image-20220503125941169.png" alt="image-20220503125941169" style="zoom:50%;" />

​	然后单词分割完毕后，对每个单词进行类型判断，通过DFA的状态转移表f对字符串序列逐个处理，每处理一个更新当前状态，直至状态为-1即找不到下一个状态为止，判断当前是否是因为未知符号导致的中断，将已经识别的字符串作为一个token识别其属于关键字，操作符，界符，常数和标识符的哪一种。具体算法执行流程如下：

<img src="../blogImages/README/image-20220503130118347.png" alt="image-20220503130118347" style="zoom:50%;" />

### （5）其他非关键函数

- 包括：打印NFA、打印DFA、关键词、界符等的判断函数、状态重命名函数、过滤源程序函数等等。



## 1.7 实例分析





# 2. LR(1)语法分析

## 2.1 基本原理



## 2.2 设计思路

## 2.3 文件结构

## 2.4 算法流程

## 2.5 语法规则

## 2.6 具体实现

## 2.7 实例分析



# 3. 课设总结

语法分析的作用：识别单词符号**序列**是否是给定文法的正确程序

## LR分析法

根据当前分析栈中的符号串 和 向右顺序查看输入串的k个符号 就可以唯一地 确定分析器的动作使移进还是规约 和 用哪个产生式规约。因而能够唯一地确定句柄

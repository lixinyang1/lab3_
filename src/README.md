## README

lab2的基本框架就是`sa.cpp` / `sa.h`，只不过我还没在main里面调用，也还不知道正确性，因为没法测试。

#### TODO：

（工作量不一定准确）

* `sysy.y`: 语法分析调用AST（工作量较大，大在思考，如果知道怎么处理就没啥问题）
* `ast.h`: 补充ast节点定义，对有vector的节点补充方法（工作量小）
* `ast.cpp`: 绘制AST (工作量中/较大)
* `sa.h`: 补充符号表类的方法（工作量很小）
* `sa.cpp`: 补充不同类型节点的类型检查（工作量中）



#### 实现细节：

除了2和4，其他任务你需要先提前了解：

1. 不同类型节点存的什么东西(`ast.h`)
2. 变量`varType`和函数`FuncType`的类型分别是怎么表示的(`ast.h`)

4和5请先阅读lab2实验指南



##### 1 语法分析调用AST

需要注意的几点：

* 对于int a, b = 1, c 这种连续的Decl，请依次存入他们的name, type, 和assignStmtNode
  * 有赋值的assignStmtNode可以在类似`VarDef : Ident Assign InitVal`的时候创建一个TreeAssignStmt节点
  * 没赋值的可以考虑在类似`VarDef : Ident {"[" INT_CONST "]"} `的时候创建一个TreeVarExpStmt节点，或者你自己在继承一个没有
    update:int a,b=1,c这种一律视为赋值，如int a视为a=$其中$的type类型为ALL
  任何用的临时节点类型，反正我后面lab2我可以区分
  * 将这些节点依次推入assignStmtNodes
* 其他问题遇到了再说吧，一时半会儿想不出来



##### 2 补充ast节点定义

需要继承的节点类型和包含的变量我都已经列出来了，可以直接在我框架上面接着写，也可以自己写，但继承关系、名字、变量最好按照我的来，而且lab2中类型检查的递归调用也是基于基类和框架提供的`as`函数实现的，可以自己实现，告诉我接口怎么用就行



##### 3 绘制AST

工作量取决于想多好看，但有一点，不要在AST里面打印没用的信息（比如括号什么的，按照给的那些节点定义打印应该就行）

如果实在嫌麻烦，就把我lab2的遍历框架搬过去



##### 4 补充符号表类的方法

Lookup没找到请将返回类型中的Type设为FAIL

没什么好说的，我自己补补完都行，几行代码的事



##### 5 补充不同类型节点的类型检查

我自己实现了其中两个例子供作参考。

需要注意的是，我把查看变量或函数在不在符号表中也放在type_check中实现，调用lookup没找到会返回FAIL



#### 

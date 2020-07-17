# oclint-growing
oclint version 0.15 support , growing rule add

基于oclint的growing规则添加，关于oclint的使用请参考 https://github.com/oclint/oclint

# 如何使用

- 使用前先安装 oclint tool
1. 先拉下专案
```c
brew tap growingio/homebrew-oclint
```
2. 然后安装库
```c
brew install oclint-growing
```

- 打开OCLint工程，添加你的代码或者framework到工程中

1. 如果你不是测试部分代码，也不是测试某一个framework，而是创建一个工程，请同样创建 CodeChecker 到你自己的工程，脚本内容一致，然后运行。
2. Test.m文件中包含了错误示例，你可以修改该文件内容，或者注释来编译通过
3. MyFramework为外部framework，这里展示如何检查外部framework，代码请自行添加

- 运行 CodeChecker ，显示代码错误

# CodeChecker配置

`GIO_METHOD_ALLOW_UPPERCASE` 允许的method前缀大写 例如URL，传入"URL,GIO",将以','拆分
`GIO_CATEGORY_PREFIX` 必须的分类前缀，默认Growing
`GIO_CLASS_PREFIX` 必须的类前缀，默认Growing
`GIO_STATIC_NSSTRING_PREFIX` static NSString的变量前缀，默认为kGrowing

# 声明

仅用于内部使用



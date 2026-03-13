cmake_minimum_required(VERSION 3.16)
project(LingJianPinyin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 为 Qt 启用 AUTOMOC，这样带有 Q_OBJECT 的类（如 CandidateView）
# 会自动生成并编译 moc 源文件，避免 vtable 未定义引用的问题。
set(CMAKE_AUTOMOC ON)

option(BUILD_UI "Build standalone UI process" ON)

add_subdirectory(src)


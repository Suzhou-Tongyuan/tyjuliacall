import os

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "tyjuliasetup")
TARGET = os.path.join(ROOT, "julia_src_binding.py")

# 递归遍历所有文件夹，对于后缀为jl的文件
# 读取其中的内容，作为字符串写入目标文件中


def read_jl_file(file):
    file_name = os.path.basename(file)
    code_name, _ = os.path.splitext(file_name)
    with open(file, "r", encoding="utf-8") as f:
        return code_name, f.read()


with open(TARGET, "w", encoding="utf-8") as f:
    f.write("# this file is auto-generated from build.py\n")
    f.write("JL_SRC = {}\n\n")
    for root, dirs, files in os.walk(ROOT):
        for file in files:
            if file.endswith(".jl"):
                print(f"writing {file} into {TARGET}...")
                code_name, code = read_jl_file(os.path.join(root, file))
                f.writelines(f'JL_SRC["{code_name}"] = r"""\n')
                f.writelines([code, "\n"])
                f.writelines('"""\n')

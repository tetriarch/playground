import os
import subprocess
import sys

patterns = [["vertex", "_vert.glsl"], ["fragment", "_frag.glsl"]]


def contains(string: str, pattern: str):
    start = string.find(pattern)

    if start != -1:
        end = start + len(pattern)
        return True, end
    else:
        return False, -1


def compile_shader(source: str, target: str, shader_stage: str):
    stage_argument = "-fshader-stage=" + shader_stage
    result = subprocess.run(
        ["glslc", stage_argument, source, "-o", target], check=False
    )
    if result.returncode != 0:
        print("[ERROR]: failed to compile", source, target, shader_stage)
        return 1
    return 0


def execute(source_dir: str, target_dir: str):
    if not os.path.exists(source_dir):
        print("[ERROR]: source_dir", source_dir, "does not exist")
        return 1

    os.makedirs(target_dir, exist_ok=True)

    for file in os.listdir(source_dir):
        if not os.path.isfile(os.path.join(source_dir, file)):
            continue

        matched = False

        for k, v in patterns:
            match, end = contains(file, v)
            if match and end == len(file):
                raw_file_name = file[:-5]
                old_file_name = os.path.join(source_dir, file)
                new_file_name = os.path.join(target_dir, raw_file_name + ".spv")

                result = compile_shader(old_file_name, new_file_name, k)
                if result != 0:
                    return 1
                matched = True
                break

        if not matched:
            print("[ERROR]: could not match", file, "to any pattern")
            return 1

    return 0


if len(sys.argv) != 3:
    print("Usage: compile_shader.py <source_dir> <target_dir>")
    sys.exit(1)

sys.exit(execute(sys.argv[1], sys.argv[2]))

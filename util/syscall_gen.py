import sys as _sys

if len(_sys.argv) != 2:
    print("Incorrect number of arguments")
    exit(-1)
elif ["c", "h", "asm"].index(_sys.argv[1]) == -1:
    print("Invalid filetype %s" % _sys.argv[1])
    exit(-1)

print("\t\033[32;1mGenerating\033[0m include/gen/syscall_gen.%s" % _sys.argv[1])

syscall_list = []

def syscall(name, args="void"):
    syscall_list.append({ "name": name, "args": args })

syscall("write", "char c")
syscall("fork", "uint64_t flags, struct callee_regs *regs, virtaddr_t return_addr")
syscall("exit", "int code")
syscall("sched_yield")

decls = [
    "#include \"types.h\"",
    "#include \"proc.h\"",
    "",
]
defs = [
    "#define ENOSYS 0xFFFFFFFFFFFFFFFFLL",
    "#define NUM_SYSCALLS %d" % len(syscall_list)
]
table = [
    "syscall_t syscall_table[NUM_SYSCALLS] = {"
]
asm_defs = [
    "%define ENOSYS 0xFFFFFFFFFFFFFFFF",
    "%%define NUM_SYSCALLS %d" % len(syscall_list)
]

for i in range(0, len(syscall_list)):
    sys = syscall_list[i]
    defs.append("#define SYSCALL_%s %d" % (sys["name"].upper(), i))
    table.append("\t[SYSCALL_%s] = (syscall_t)syscall_%s," % (sys["name"].upper(), sys["name"]))
    decls.append("int64_t syscall_%s(%s);" % (sys["name"], sys["args"]))
    asm_defs.append("%%define SYSCALL_%s %d" % (sys["name"].upper(), i))

table.append("};")

defs = "\n".join(defs)
table = "\n".join(table)
decls = "\n".join(decls)
asm_defs = "\n".join(asm_defs)

h_out = """#pragma once

%s

%s

extern syscall_t syscall_table[NUM_SYSCALLS];
""" % (defs, decls)

c_out = """%s
""" % table

asm_out = """%s
""" % asm_defs

out_data = { "c": c_out, "h": h_out, "asm": asm_out }

prefix = "include/gen/syscall_gen"
path = prefix + "." + _sys.argv[1]

target_file = open(path, "w")
target_file.write(out_data[_sys.argv[1]])
target_file.close()

import sys as _sys

print("\t\033[32;1mGenerating\033[0m syscalls")

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

if len(_sys.argv) != 2:
    print("Incorrect number of arguments")
    exit(-1)

prefix = "include/gen/%s" % _sys.argv[1]

h_path = prefix + ".h"
c_path = prefix + ".c"
asm_path = prefix + ".asm"

h_file = open(h_path, "w")
c_file = open(c_path, "w")
asm_file = open(asm_path, "w")

h_file.write(h_out)
c_file.write(c_out)
asm_file.write(asm_out)

h_file.close()
c_file.close()
asm_file.close()

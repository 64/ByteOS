syscall_list = []

def syscall(name, args="void"):
    syscall_list.append({ "name": name, "args": args })

syscall("write", "char c")
syscall("fork", "uint64_t flags, struct callee_regs *regs, virtaddr_t return_addr")
syscall("exit", "int code")
syscall("sched_yield")

decls = []
defs = [
    "#define ENOSYS 0xFFFFFFFFFFFFFFFF",
    "#define NUM_SYSCALLS %d" % len(syscall_list)
]
table = [
    "syscall_t syscall_table[NUM_SYSCALLS] = {"
]
asm_defs = [
    "%%define ENOSYS 0xFFFFFFFFFFFFFFFF",
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

print("--- Defines ---")
print(defs)
print("\n--- Declarations ---")
print(decls)
print("\n--- Table ---")
print(table)
print("\n--- Assembly Defines ---")
print(asm_defs)


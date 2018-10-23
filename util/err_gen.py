import sys as _sys

if len(_sys.argv) != 1:
    print("Incorrect number of arguments")
    exit(-1)

print("\t\033[32;1mGenerating\033[0m include/gen/err_gen.py")

err_list = []
next_value = 1

def error(name, desc):
    global next_value
    err_list.append({ "name": name, "desc": desc, "value": next_value })
    next_value += 1

error("EINVAL", "invalid")
error("ENOTDIR", "not a valid directory")
error("ENOTSUP", "not supported")
error("EBUSY", "busy")
error("ENOENT", "no directory entry")

defs = [
    "#pragma once",
    ""
]

strerror = [
    "",
    "static inline const char *strerror(err_t e)",
    "{",
    "\tconst char *rv;",
    "\t",
    "\tswitch (e) {",
    "\t\tcase 0: rv = \"no error\"; break;",
    "\t\tdefault: rv = \"unknown error\"; break;"
]

for err in err_list:
    defs.append("#define %s (-%dL)" % (err["name"], err["value"]))
    strerror.append("\t\tcase %s: rv = \"%s\"; break;" % (err["name"], err["name"] + " (" + err["desc"] + ")"))

strerror.append("\t}")
strerror.append("")
strerror.append("\treturn rv;")
strerror.append("}")

out = "\n".join(defs) + "\n".join(strerror)

path = "include/gen/err_gen.h"

target_file = open(path, "w")
target_file.write(out)
target_file.close()

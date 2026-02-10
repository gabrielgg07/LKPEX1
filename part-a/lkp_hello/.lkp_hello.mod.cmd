savedcmd_lkp_hello.mod := printf '%s\n'   lkp_hello.o | awk '!x[$$0]++ { print("./"$$0) }' > lkp_hello.mod

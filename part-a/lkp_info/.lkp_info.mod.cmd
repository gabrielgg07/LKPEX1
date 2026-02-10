savedcmd_lkp_info.mod := printf '%s\n'   lkp_info.o | awk '!x[$$0]++ { print("./"$$0) }' > lkp_info.mod

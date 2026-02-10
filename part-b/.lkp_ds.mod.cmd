savedcmd_lkp_ds.mod := printf '%s\n'   lkp_ds.o | awk '!x[$$0]++ { print("./"$$0) }' > lkp_ds.mod

set breakpoint pending on
b unix_iface.c:368
commands
  set ret=-1
  c
end
run
quit $_exitcode

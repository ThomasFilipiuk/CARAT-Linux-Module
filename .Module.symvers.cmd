cmd_/files10/cs446/rt116146/mini-linux/module/Module.symvers := sed 's/\.ko$$/\.o/' /files10/cs446/rt116146/mini-linux/module/modules.order | scripts/mod/modpost    -o /files10/cs446/rt116146/mini-linux/module/Module.symvers -e -i Module.symvers   -T -

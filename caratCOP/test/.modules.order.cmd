cmd_/home/tnf9402/carat-linux/mini-linux/module/caratCOP/test/modules.order := {   echo /home/tnf9402/carat-linux/mini-linux/module/caratCOP/test/test.ko;   echo /home/tnf9402/carat-linux/mini-linux/module/caratCOP/test/carat-cop.ko; :; } | awk '!x[$$0]++' - > /home/tnf9402/carat-linux/mini-linux/module/caratCOP/test/modules.order

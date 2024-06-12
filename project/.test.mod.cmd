savedcmd_/home/guhyeon/test/tasktest/test.mod := printf '%s\n'   main.o task.o | awk '!x[$$0]++ { print("/home/guhyeon/test/tasktest/"$$0) }' > /home/guhyeon/test/tasktest/test.mod

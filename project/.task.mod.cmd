savedcmd_/home/guhyeon/test/tasktest/task.mod := printf '%s\n'   task.o | awk '!x[$$0]++ { print("/home/guhyeon/test/tasktest/"$$0) }' > /home/guhyeon/test/tasktest/task.mod

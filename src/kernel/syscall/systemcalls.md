
# System calls

|system call|name   |args|return
|-----------|-------|-|-
|AX = 0     |Read   |EBX = file dis<br>ESI = buffer<br>ECX count|EAX -1 if error and count if good
|AX = 1     |Write  |EBX = file dis<br>ESI = buffer<br>ECX count|EAX -1 if error and count if good
|AX = 2     |Exit   |EBX = exit code|&nbsp;
|AX = 3     |Open   |ESI = path|EBX -1 if error and file dis if good
|AX = 4     |Close  |EBX = file dis|EAX -1 if error and 0 if good
|AX = 50    |Map    |ECX = size|EAX returns a pointer
|AX = 51    |UnMap  |EBX = point|&nbsp;

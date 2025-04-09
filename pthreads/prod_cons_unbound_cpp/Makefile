include ../../common/Makefile

# Simulation
# DEFS += -DSIMULATION
# ARGS = 1000 3 1 -2 -1

# WebServer
DEFS += -DWEBSERVER
ARGS=8080
HOST=localhost
PORT=8080
CONN=200
RATE=50
REQS=3
NUMS=1234,-7777,abc,+09,--5,1il1,40939419221,,581501921907029202329135354687,2
APP=fact
URI=/$(APP)?number=$(NUMS)
TIMEOUT=3
DEPS+=httperf httpie

.PHONY: stress crun
stress: # Stress test
	httperf --server $(HOST) --port $(PORT) --num-conns $(CONN) --rate $(RATE) --num-call $(REQS) --uri $(URI) --timeout $(TIMEOUT)

crun: # Client run
	http $(HOST):$(PORT)$(URI)

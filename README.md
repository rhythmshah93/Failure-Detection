# Failure Detection
Simulates a network of nodes, with some of them failinglate on. The algorithm uses heartbeat messages with a gossip based protocol to detect and maintain a consistent view of the network.
 
To run the code, run the makefile and before running it again, pleaase run make clean as otherwise endpoints file won't be deleted and the program will not run correctly.

The heartbeat of a node is decided by the node itself. A node increments it's heartbeat after every one second so that duplicate heartbeat messages can be detected. Thus heartbeat of a node generally has the value of its local time itself.

TEST CASES:

CASE 1: ./p4 5 3 5 4 2 3 89 25
output for this case is:
list1
OK
1 25
2 5
3 24
4 2
5 24

list2
FAIL
1 5
2 6
3 5
4 2
5 6

list3
OK
1 24
2 5
3 25
4 2
5 24

list4
FAIL
1 2
2 2
3 1
4 3
5 2

list5
OK
1 23
2 5
3 24
4 2
5 25
Conclusion: CONVERGENCE
In this case, as the T is long enough for the nodes to have failed and the information to have propagated to all the nodes, convergence is observed. All the nodes detect the failures. The heartbeat of failed node is the time before the node fails. For failed nodes, the maximum heartbeat of a node is the heartbeat at time it fials as after failure it stops listening and doesn't update any information.

CASE 2: ./p4 5 3 5 4 2 10 89 21
output for this case is:
list1
OK
1 21
2 19
3 19
4 9
5 20

list2
FAIL
1 19
2 20
3 19
4 9
5 19

list3
OK
1 20
2 19
3 21
4 9
5 20

list4
FAIL
1 9
2 9
3 9
4 10
5 10

list5
OK
1 20
2 19
3 20
4 9
5 21
Conclusion: NO CONVERGENCE
Here, the T parameter is 21 while time between failures(P) is 10 and F(time after which a node is considered dead) is 4. Hence the second node will fail at time 20 but other nodes will not detect the failure because they will exit before F seconds can be measured as T is 21. Hence there will be no convergence in thiss case.Node 2 which is failing at time 20 will stop listening as well as sending messages which can is reflected in last heartbeat of node 2 for other live nodes which is 19. Hence if the program is allowd to run longer convergence will be seen as other nodes detect node 2's failure.

CASE 3:./p4 5 3 5 4 3 3 89 25
Output for this case is:
list1
FAIL
1 9
2 5
3 9
4 2
5 9

list2
FAIL
1 5
2 6
3 6
4 2
5 6

list3
OK
1 8
2 5
3 25
4 2
5 24

list4
FAIL
1 2
2 3
3 3
4 3
5 3

list5
OK
1 8
2 5
3 24
4 2
5 25
Conclusion: Here n(number of processes = 5) - B(number of bad nodes = 3) < gossipb (number of nodes to send messages = 3). hence in this case as there are not enough nodes to satiisfy gossipb, live nodes should send messages to all the remaining live nodes. From output it can be seen that nodes 4, 2 and 1 fail in that order. Hence after failure of node 1 node 3 and 5 send messages to each other only as they are the only ones alive.

GENERAL CASE: Uniqueness
Whenever a node picks nodes to send messages to, it should not pick a same node twice, i.e all the nodes should be unique. Similarly, once a node has failed, it cannot fail again. Hence while failing nodes, this should be taken care of. This is done by using an array which keeps track of which nodes have be selected so that we acheive unique selections. 

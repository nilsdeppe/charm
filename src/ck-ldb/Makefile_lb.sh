#!/bin/sh
LOADBALANCERS="DummyLB ComboCentLB RandCentLB RefineLB RefineKLB  RefineCommLB GreedyLB GreedyCommLB GreedyAgentLB GridCommLB Comm1LB OrbLB RecBisectBfLB MetisLB PhasebyArrayLB RotateLB NeighborLB NeighborCommLB WSLB TopoLB RefineTopoLB TopoCentLB HybridLB HbmLB BlockLB"

out="Make.lb"

echo "# Automatically generated by script Makefile_lb.sh" > $out
echo "#  by" `id` >>$out
echo "#  at" `hostname` >>$out
echo "#  on" `date` >> $out
echo "LOADBALANCERS=\\" >> $out
for bal in $LOADBALANCERS 
do 
	echo "   \$(L)/libmodule$bal.a \\" >> $out 
done
echo "   manager.o" >> $out
echo >> $out

for bal in $LOADBALANCERS 
do 
	dep=""
	[ -r libmodule$bal.dep ] && dep="cp libmodule$bal.dep "'$'"(L)/"
        manager=""
        [ $bal = 'GreedyCommLB' ] && manager="manager.o"
        [ $bal = 'GridCommLB' ] && manager="manager.o"
	cat >> $out << EOB 
$bal.def.h: $bal.decl.h

$bal.decl.h: $bal.ci charmxi
	\$(CHARMXI) $bal.ci

\$(L)/libmodule$bal.a: $bal.o $manager
	\$(CHARMC) -o \$(L)/libmodule$bal.a $bal.o $manager
	$dep

EOB
done

echo "" >  EveryLB.ci
echo "module EveryLB {" >> EveryLB.ci
for bal in $LOADBALANCERS
do
	echo "   extern module $bal;" >> EveryLB.ci
done
echo "   initnode void initEveryLB(void);" >>EveryLB.ci
echo "};" >> EveryLB.ci

echo "# used for make dependes" >>$out
echo "LB_OBJ=EveryLB.o \\" >>$out
for bal in $LOADBALANCERS
do
	echo "    $bal.o \\" >>$out
done
echo "    manager.o" >> $out
cat >> $out <<EOB

EveryLB.def.h: EveryLB.decl.h

EveryLB.decl.h: EveryLB.ci charmxi
	\$(CHARMXI) EveryLB.ci

\$(L)/libmoduleEveryLB.a: \$(LB_OBJ)
	\$(CHARMC) -o \$(L)/libmoduleEveryLB.a \$(LB_OBJ)
	cp libmoduleEveryLB.dep \$(L)/
EOB

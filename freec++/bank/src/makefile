#########################################
# ��ע�⣬����ʱ��ȷ�����¼��㣺
# 1���Ѱ�װoracle�Ŀͻ��˻�oracle����˳��򣬲��ұ��밲װ��OCI������
# 2�����ú�ORACLE_HOME��������
#
#########################################

# oracleͷ�ļ�·��
ORAINCL = -I$(ORACLE_HOME)/rdbms/public

# oracle���ļ�·��
ORALIB =  -L$(ORACLE_HOME)/lib -L.

# oracle��oci��
ORALIBS = -lclntsh

# CFLAGS = -O2
# CFLAGS = -O2 -Wall

CFLAGS = -g -Wall -Wno-write-strings -Wno-unused-variable

all:    bankserver bankclient

bankserver: bankserver.cpp _ooci.h _ooci.cpp bank.h bank.cpp
	g++ $(CFLAGS) -o bankserver bankserver.cpp $(ORAINCL) $(ORALIB) $(ORALIBS) _public.cpp _ooci.cpp bank.cpp -lm -lc
	cp bankserver ../bin/.

bankclient: bankclient.cpp 
	g++ $(CFLAGS) -o bankclient bankclient.cpp _public.cpp -lm -lc
	cp bankclient ../bin/.

clean:
	rm -rf bankserver bankclient
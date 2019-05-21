//
// ��������ʾ����Ʒ���в���10����¼��
//

#include "_mysql.h"

// �������ڲ������ݵĽṹ������е��ֶζ�Ӧ
struct st_GOODS
{
  long id;          // ��Ʒ��ţ���long�������Ͷ�Ӧmysql��С����bigint
  char name[31];    // ��Ʒ���ƣ���char��Ӧmysql��varchar��ע�⣬�����ֶεĳ�����30��char����ĳ�����31��Ҫ��C���ԵĽ�����
  double sal;       // ��Ʒ�۸���double�������Ͷ�Ӧmysql��С����decimal
  char btime[20];   // ���ʱ�䣬��char��Ӧmysql��datetime����ʽ������SQL�����ָ����������ָ��Ϊ'%%Y-%%m-%%d %%h:%%i:%%s'
} stgoods;

int main(int argc,char *argv[])
{
  // ���ݿ����ӳ�
  connection conn;
  
  // �������ݿ⣬����ֵ0-�ɹ�������-ʧ��
  // ʧ�ܴ�����conn.m_cda.rc�У�ʧ��������conn.m_cda.message�С�
  if (conn.connecttodb("193.112.167.234,test,testpwd,testdb,3306","gbk") != 0)
  {
    printf("connect database failed.\n%s\n",conn.m_cda.message); return -1;
  }
  
  // SQL���Բ�����
  sqlstatement stmt(&conn);
  
  // ׼���������ݵ�SQL������Ҫ�жϷ���ֵ
  stmt.prepare("\
    insert into goods(id,name,sal,btime) \
               values(:1,:2,:3,str_to_date(:4,'%%Y-%%m-%%d %%h:%%i:%%s'))");
  // ΪSQL������������ĵ�ַ
  stmt.bindin(1,&stgoods.id);
  stmt.bindin(2, stgoods.name,30);
  stmt.bindin(3,&stgoods.sal);
  stmt.bindin(4, stgoods.btime,19);

  // ģ����Ʒ���ݣ�����в���10��������Ϣ
  for (int ii=1;ii<=10;ii++)
  {
    // �ṹ�������ʼ��
    memset(&stgoods,0,sizeof(stgoods));

    // Ϊ�ṹ��ı���ָ��ֵ
    stgoods.id=ii;
    sprintf(stgoods.name,"��Ʒ����%02d",ii);
    stgoods.sal=ii*2.11;
    strcpy(stgoods.btime,"2018-03-01 12:25:31");

    // ÿ��ָ��������ֵ��ִ��SQL��䣬һ��Ҫ�жϷ���ֵ��0-�ɹ�������-ʧ�ܡ�
    if (stmt.execute() != 0)
    {
      printf("stmt.execute() failed.\n%s\n%s\n",stmt.m_sql,stmt.m_cda.message); return -1;
    }

    printf("insert ok(id=%d).\n",ii);
  }

  printf("insert table goods ok.\n");

  // �ύ���ݿ�����
  conn.commit();

  return 0;
}


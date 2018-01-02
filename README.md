# sMail
简易邮箱，使用Qt、C++实现

简单的实现了POP和SMTP协议，能实现基本的收信发信，但目前不支持ssl/tls
sMailDB类管理用户数据库，并构建User。
User类是中心，包含了一个用户的全部信息和所能执行的功能，
MailModel 用户邮件模型类，
MIME系列类用来构造邮件，但管用的只有MIMEText，MIMEVideo和MIMEApplication，
MailPraser类用来解析邮件。
界面类将用户的请求转换为User类的某个方法调用，

注：如果想要更换用户，请直接删除根目录下的sMail.db,然后你就可以重新登录了

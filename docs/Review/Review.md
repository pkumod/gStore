#gStore代码Review记录

##gStore0.7.1.dev(2018-09-30)

###总结

1. 目前李荆和苏勋斌安排的任务基本完成，逻辑正确，可以提交到dev版本；


2. 存在的主要问题
	1. 代码注释较少，如
	
			else if(type == "change_psw")
			{
			pthread_rwlock_wrlock(&users_map_lock);
			std::map<std::string, struct User *>::iterator iter;
			iter = users.find(username2);
		
			if(iter == users.end())
			{
				string error = "username not exist, change password failed.";
				string resJson = CreateJson(916, error, 0);
				*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json" << "\r\n\r\n" << resJson;
				pthread_rwlock_unlock(&users_map_lock);
		
				return false;
				
			}
			else
			{
				iter->second->setPassword(password2);
			}
			pthread_rwlock_unlock(&users_map_lock);

			}

	如此处就应该叫上必要的代码

 	

  2. 缺少必要的try catch异常捕获及处理代码，如
			
			else if(type == "change_psw")
			{
			pthread_rwlock_wrlock(&users_map_lock);
			std::map<std::string, struct User *>::iterator iter;
			iter = users.find(username2);
		
			if(iter == users.end())
			{
				string error = "username not exist, change password failed.";
				string resJson = CreateJson(916, error, 0);
				*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json" << "\r\n\r\n" << resJson;
				pthread_rwlock_unlock(&users_map_lock);
		
				return false;
				
			}
			else
			{
				iter->second->setPassword(password2);
			}
			pthread_rwlock_unlock(&users_map_lock);

			}
  此处代码如果抛异常的话，那么`pthread_rwlock_unlock`是否可以执行？如果不能执行的话，是否会导致锁不能解锁？

###李荆的代码Rewiew情况

1. 错误代码维护
>基本没有问题，有如下可以改进的地方
>>1. 所有操作的`success`都应该是一个code，因为用户只需要关注是否成功，成功了的话，至于`msg`可以由客户自己去组织,当然也可以我们提供，举个例子

		json.put("errorcode",0);
        json.put("errormsg","load database successful")

>>2 可以将错误代码表做成md文件放在项目文件里面，方便使用者查询

2.JSON封装
>目前采用的是boost的json相关方法，当然是可以，但我们需要考虑的是，目前gStore对boost的依赖太强（个人感觉），而boost又是一个重量级的库（安装文件达1G以上），这样的话，系统耦合度太高不利于将来的拓展，我推荐采用腾讯的开源[RapidJSON](http://rapidjson.org "RapidJSON官网"),可以访问[GitHub](https://github.com/Tencent/rapidjson/ "GitHub")，目前号称是性能最高的c++JSON库，类似于Java的JSON库[fastJSON](https://github.com/alibaba/fastjson "fastJSON github")

3.密码安全
>目前由于修改的user的password是以`map`的方式存储在内存中，当ghttp重启之后，password又会还原，所以需要整改，邹老师意见是在数据库中创建一个`system.db`数据库，用于保存系统信息和参数数据，此方案可行


###苏勋斌代码Review情况
1. ghttp不带参数启动
>设置了默认值，代码逻辑不存在问题

2.ghttp关闭
>增加了`shutdown`指令，且在ghttp的servlet里面提供了stop接口供外部接口调用，使用了外部接口的远程关闭ghttp功能

3.ghttp数据库状态维护
>新增或重构了`stop`，`shop`，`drop`，`checkpoint`，`checkall`等接口，基本满足需求，当需要增加相关接口描述，建议在[API.md](..\API.md)增加相关描述





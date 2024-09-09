/*
# Filename: GstoreConnector.js
# Author: suxunbin
# Last Modified: 2019-5-20 14:10
# Description: http api for nodejs
*/

const request = require('request-promise');

let strUrl = ""

class GstoreConnector {
    constructor(ip = '', port, httpType = 'ghttp', username = '', password = '') 
    {
        if (ip == 'localhost')
            this.serverIP = '127.0.0.1';
        else
            this.serverIP = ip;
        this.serverPort = port;
        this.Url = 'http://' + this.serverIP + ':' + this.serverPort.toString() + '/';
        if (httpType == 'grpc')
            this.Url = this.Url + 'grpc/api/';
        this.username = username;
        this.password = password;
    }

    async Get(strUrl = '') 
    {
        strUrl = this.Url + encodeURIComponent(strUrl);
        const res = await request({uri: strUrl, method: 'GET', json: true});
        return res;
    }

    async Post(strPost = '') 
    {
        strUrl = this.Url;
        const res = await request({
            uri: strUrl, 
            method: 'POST', 
            json: true, 
            body: strPost,  
            headers:{
                'Content-Type':'application/x-www-form-urlencoded',
                // 'Content-Length':strPost.length
            }
        });
        return res;
    }

    async check(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=check";
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"check\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async login(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=login&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"login\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async testConnect(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=testConnect&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"testConnect\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async getCoreVersion(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=getCoreVersion&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"getCoreVersion\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async ipmanage(type = '', ip_type = '', ips = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=ipmanage&type=" + type + "&ip_type=" + ip_type + "&ips=" + ips  + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"ipmanage\", \"type\": \"' + type + '\", \"ip_type\": \"' + ip_type + '\", \"ips\": \"' + ips + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async upload(filepath = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=upload&filepath=" + filepath + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"upload\", \"filepath\": \"' + filepath + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async download(filepath = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=download&filepath=" + filepath + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"download\", \"filepath\": \"' + filepath + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async stat(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=stat&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"stat\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async shutdown(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=shutdown&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"shutdown\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async show(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=show&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"show\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async load(db_name = '', csr = '0', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=load&db_name=" + db_name + "&csr=" + csr + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"load\", \"db_name\": \"' + db_name + '\", \"csr\": \"' + csr + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async unload(db_name = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=unload&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"unload\", \"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async monitor(db_name = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=monitor&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"monitor\", \"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async build(db_name = '', db_path = '', async = '', callback = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=build&db_name=" + db_name + "&db_path=" + db_path + "&async=" + async + "&callback=" + callback + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"build\", \"db_name\": \"' + db_name + '\", \"db_path\": \"' + db_path + "\", \"async\": \"" + async + "\", \"callback\": \"" + callback + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async drop(db_name = '', is_backup , request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            var strUrl;
            if(is_backup)
                strUrl = "?operation=drop&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password + "&is_backup=true";
            else
                strUrl = "?operation=drop&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password + "&is_backup=false";
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            var strPost;
            if(is_backup)
                strPost = '{\"operation\": \"drop\", \"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"is_backup\": \"true\"}';
            else
                strPost = '{\"operation\": \"drop\", \"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"is_backup\": \"false\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async backup(db_name = '', backup_path = '', async = '', callback = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=backup&db_name=" + db_name + "&backup_path=" + backup_path + "&async=" + async + "&callback=" + callback + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"backup\", \"db_name\": \"' + db_name + '\", \"backup_path\": \"' + backup_path + "\", \"async\": \"" + async + "\", \"callback\": \"" + callback + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async backuppath(db_name = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=backuppath&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"backuppath\", \"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async restore(db_name = '', backup_path = '', async = '', callback = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
            {
                const strUrl = "?operation=restore&db_name=" + db_name + "&backup_path=" + backup_path + "&async=" + async + "&callback=" + callback + "&username=" + this.username + "&password=" + this.password;
                const res = this.Get(strUrl);
                return res;
            }
            else if (request_type == 'POST')
            {
                const strPost = '{\"operation\": \"restore\", \"db_name\": \"' + db_name + '\", \"backup_path\": \"' + backup_path + "\", \"async\": \"" + async + "\", \"callback\": \"" + callback + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
                const res = this.Post(strPost);
                return res;
            }
    }

    async query(db_name = '', format = '' , sparql = '' , request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=query&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&format=" + format + "&sparql=" + sparql;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"query\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"format\": \"' + format + '\", \"sparql\": \"' + sparql + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async exportDB(db_name = '', db_path = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=export&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&db_path=" + db_path;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"export\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"db_path\": \"' + db_path + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async batchInsert(db_name = '', file = '', dir = '', async = '', callback = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=batchInsert&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password + "&file=" + file + "&dir=" + dir + "&async=" + async + "&callback=" + callback;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"batchInsert\", \"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"file\": \"' + file + '\", \"dir\": \"' + dir + '\", \"async\": \"' + async + '\", \"callback\": \"' + callback + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async batchRemove(db_name = '', file = '', async = '', callback = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=batchRemove&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password + "&file=" + file + "&async=" + async + "&callback=" + callback;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"batchRemove\", \"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"file\": \"' + file + '\", \"async\": \"' + async + '\", \"callback\": \"' + callback + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async rename(db_name = '', new_name = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=rename&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&new_name=" + new_name;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"rename\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"new_name\": \"' + new_name + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async checkOperationState(opt_id = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=checkOperationState&opt_id=" + opt_id + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"checkOperationState\", \"opt_id\": \"' + opt_id + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async begin(db_name = '', isolevel = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=begin&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&isolevel=" + isolevel;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"begin\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"isolevel\": \"' + isolevel + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async tquery(db_name = '', tid = '' , sparql = '' , request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=tquery&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&tid=" + tid + "&sparql=" + sparql;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"tquery\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"tid\": \"' + tid + '\", \"sparql\": \"' + sparql + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async commit(db_name = '', tid = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=commit&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&tid=" + tid;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"commit\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"tid\": \"' + tid + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async rollback(db_name = '', tid = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=rollback&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&tid=" + tid;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"rollback\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"tid\": \"' + tid + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async checkpoint(db_name = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=checkpoint&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"checkpoint\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async showuser(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=showuser&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"showuser\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async usermanage(type = '', op_username = '' , op_password = '' , request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=usermanage&type=" + type + "&username=" + this.username + "&password=" + this.password + "&op_username=" + op_username + "&op_password=" + op_password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"usermanage\", \"type\": \"' + type + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"op_username\": \"' + op_username + '\", \"op_password\": \"' + op_password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async userprivilegemanage(type = '', op_username = '' , privileges = '' , db_name = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=userprivilegemanage&type=" + type + "&username=" + this.username + "&password=" + this.password + "&op_username=" + op_username + "&privileges=" + privileges + "&db_name=" + db_name;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"userprivilegemanage\", \"type\": \"' + type + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"op_username\": \"' + op_username + '\", \"privileges\": \"' + privileges + '\", \"db_name\": \"' + db_name + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async userpassword(re_username, re_password, op_password, request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=userpassword&username=" + re_username + "&password=" + re_password + "&op_password=" + op_password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"userpassword\", \"username\": \"' + re_username + '\", \"password\": \"' + re_password + '\", \"op_password\": \"' + op_password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async funquery(funInfo, request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=funquery&username=" + this.username + "&password=" + this.password + "&funInfo=" + funInfo;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"funquery\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"funInfo\": \"' + funInfo + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async fquery(db_name, format, sparql, filename, request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=query&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&format=" + format + "&sparql=" + sparql + "&filename=" + filename;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"query\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"format\": \"' + format + '\", \"sparql\": \"' + sparql + '\", \"filename\": \"' + filename + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async funcudb(type, funInfo, request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=funcudb&username=" + this.username + "&password=" + this.password + "&type=" + type + "&funInfo=" + funInfo;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"funcudb\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"type\": \"' + type + '\", \"funInfo\": \"' + funInfo + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async funreview(funInfo, request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=funreview&username=" + this.username + "&password=" + this.password + "&funInfo=" + funInfo;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"funreview\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"funInfo\": \"' + funInfo + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async txnlog(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=txnlog&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"txnlog\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async querylogdate(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=querylogdate&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"querylogdate\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async querylog(date, pageNo, pageSize, request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=querylog&date=" + date + "&username=" + this.username + "&password=" + this.password + "&pageNo=" + pageNo + "&pageSize=" + pageSize;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"querylog\", \"date\": \"' + date + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"pageNo\": \"' + pageNo + '\", \"pageSize\": \"' + pageSize + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async accesslogdate(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=accesslogdate&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"accesslogdate\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async accesslog(date, pageNo, pageSize, request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=accesslog&date=" + date + "&username=" + this.username + "&password=" + this.password + "&pageNo=" + pageNo + "&pageSize=" + pageSize;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"accesslog\", \"date\": \"' + date + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"pageNo\": \"' + pageNo + '\", \"pageSize\": \"' + pageSize + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async addReason(type, db_name, ruleinfo) 
    {
        const strPost = '{\"operation\": \"reasonManage\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"type\": \"' + type + '\", \"db_name\": \"' + db_name + '\", \"ruleinfo\":' + ruleinfo + '}';
        const res = this.Post(strPost);
        return res;
    }

    async reasonManage(type, db_name, rulename = '') 
    {
        const strPost = '{\"operation\": \"reasonManage\", \"type\": \"' + type + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"rulename\": \"' + rulename + '\"}';
        const res = this.Post(strPost);
        return res;
    }
}

module.exports = GstoreConnector;

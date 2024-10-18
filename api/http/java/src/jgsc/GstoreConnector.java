/*
# Filename: GstoreConnector.java
# Author: suxunbin
# Last Modified: 2024-10-18 16:20
# Description: http api for java
*/
package jgsc;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLEncoder;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Map;
import java.util.Random;

public class GstoreConnector {

    public static final String defaultServerIP = "127.0.0.1";
    public static final int defaultServerPort = 9000;

    private String serverIP;
    private int serverPort;
    private String apiUrl;
    private String fileUrl;
    private String username;
    private String password;

    public GstoreConnector(String _ip, int _port, String _http_type, String _user, String _passwd) {
        if (_ip.equals("localhost")) {
            this.serverIP = GstoreConnector.defaultServerIP;
        } else {
            this.serverIP = _ip;
        }
        this.serverPort = _port;
        this.apiUrl = "http://" + this.serverIP + ":" + this.serverPort + "/";
        if ("grpc".equals(_http_type)) {
            this.fileUrl = this.apiUrl + "grpc/file";
            this.apiUrl = this.apiUrl + "grpc/api";
        } else {
            this.fileUrl = this.apiUrl + "file";
        }
        this.username = _user;
        this.password = _passwd;
    }

    public String sendGet(String strUrl) {
        StringBuffer result = new StringBuffer();
        BufferedReader in = null;

        try {
            strUrl = URLEncoder.encode(strUrl, "UTF-8");
        } catch (UnsupportedEncodingException ex) {
            throw new RuntimeException("Broken VM does not support UTF-8");
        }

        try {
            strUrl = this.apiUrl + strUrl;
            URL realUrl = new URL(strUrl);

            // open the connection with the URL
            URLConnection connection = realUrl.openConnection();

            // set request properties
            connection.setRequestProperty("accept", "*/*");
            connection.setRequestProperty("connection", "Keep-Alive");
            connection.setRequestProperty("user-agent", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1;SV1)");
            connection.setRequestProperty("Accept-Encoding", "gzip");

            // create the real connection
            connection.connect();

            // get all response header fields
            Map<String, List<String>> map = connection.getHeaderFields();

            // define BufferedReader to read the response of the URL
            in = new BufferedReader(new InputStreamReader(connection.getInputStream(), "utf-8"));
            String line;
            while ((line = in.readLine()) != null) {
                result.append(line + "\n");
            }
        } catch (Exception e) {
            System.out.println("error in get request: " + e);
            e.printStackTrace();
        }

        // use finally to close the input stream
        finally {
            try {
                if (in != null) {
                    in.close();
                }
            } catch (Exception e2) {
                e2.printStackTrace();
            }
        }
        return result.toString();
    }

    public String sendPost(String strPost) {
        String strUrl = "";
        PrintWriter out = null;
        StringBuffer result = new StringBuffer();
        BufferedReader in = null;

        try {
            strUrl = URLEncoder.encode(strUrl, "UTF-8");
        } catch (UnsupportedEncodingException ex) {
            throw new RuntimeException("Broken VM does not support UTF-8");
        }

        try {
            strUrl = this.apiUrl;
            URL realUrl = new URL(strUrl);

            // open the connection with the URL
            URLConnection connection = realUrl.openConnection();

            // set request properties
            connection.setRequestProperty("accept", "*/*");
            connection.setRequestProperty("connection", "Keep-Alive");
            connection.setRequestProperty("user-agent", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1;SV1)");
            connection.setRequestProperty("Accept-Encoding", "gzip");

            connection.setDoOutput(true);
            connection.setDoInput(true);
            out = new PrintWriter(connection.getOutputStream());
            out.print(strPost);
            out.flush();

            // get all response header fields
            Map<String, List<String>> map = connection.getHeaderFields();

            // define BufferedReader to read the response of the URL
            in = new BufferedReader(new InputStreamReader(connection.getInputStream(), "utf-8"));
            String line;
            while ((line = in.readLine()) != null) {
                result.append(line + "\n");
            }
        } catch (Exception e) {
            System.out.println("error in post request: " + e);
            e.printStackTrace();
        }

        // use finally to close the input stream
        finally {
            try {
                if (out != null) {
                    out.close();
                }
                if (in != null) {
                    in.close();
                }
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        }
        return result.toString();
    }

    public void sendGet(String strUrl, String filename) {
        BufferedReader in = null;
        if (filename == null)
            return;

        FileWriter fw = null;
        try {
            fw = new FileWriter(filename);
        } catch (IOException e) {
            System.out.println("can not open " + filename + "!");
        }

        try {
            strUrl = URLEncoder.encode(strUrl, "UTF-8");
        } catch (UnsupportedEncodingException ex) {
            throw new RuntimeException("Broken VM does not support UTF-8");
        }

        try {
            strUrl = this.apiUrl + strUrl;
            URL realUrl = new URL(strUrl);

            // open the connection with the URL
            URLConnection connection = realUrl.openConnection();

            // set request properties
            connection.setRequestProperty("accept", "*/*");
            connection.setRequestProperty("connection", "Keep-Alive");
            connection.setRequestProperty("user-agent", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1;SV1)");
            connection.setRequestProperty("Accept-Encoding", "gzip");

            // create the real connection
            connection.connect();

            // get all response header fields
            Map<String, List<String>> map = connection.getHeaderFields();

            // define BufferedReader to read the response of the URL
            in = new BufferedReader(new InputStreamReader(connection.getInputStream(), "utf-8"));
            char chars[] = new char[2048];
            int b;
            while ((b = in.read(chars, 0, 2048)) != -1) {
                if (fw != null)
                    fw.write(chars);
                chars = new char[2048];
            }
        } catch (Exception e) {
            System.out.println("error in get request: " + e);
            e.printStackTrace();
        }

        // use finally to close the input stream
        finally {
            try {
                if (in != null) {
                    in.close();
                }
                if (fw != null) {
                    fw.close();
                }
            } catch (Exception e2) {
                e2.printStackTrace();
            }
        }
        return;
    }

    public void sendPost(String strPost, String filename) {
        String strUrl = "";
        PrintWriter out = null;
        BufferedReader in = null;

        if (filename == null)
            return;

        FileWriter fw = null;
        try {
            fw = new FileWriter(filename);
        } catch (IOException e) {
            System.out.println("can not open " + filename + "!");
        }

        try {
            strUrl = URLEncoder.encode(strUrl, "UTF-8");
        } catch (UnsupportedEncodingException ex) {
            throw new RuntimeException("Broken VM does not support UTF-8");
        }

        try {
            strUrl = this.apiUrl;
            URL realUrl = new URL(strUrl);

            // open the connection with the URL
            URLConnection connection = realUrl.openConnection();

            // set request properties
            connection.setRequestProperty("accept", "*/*");
            connection.setRequestProperty("connection", "Keep-Alive");
            connection.setRequestProperty("user-agent", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1;SV1)");
            connection.setRequestProperty("Accept-Encoding", "gzip");

            connection.setDoOutput(true);
            connection.setDoInput(true);
            out = new PrintWriter(connection.getOutputStream());
            out.print(strPost);
            out.flush();

            // get all response header fields
            Map<String, List<String>> map = connection.getHeaderFields();

            // define BufferedReader to read the response of the URL
            in = new BufferedReader(new InputStreamReader(connection.getInputStream(), "utf-8"));
            char chars[] = new char[2048];
            int b;
            while ((b = in.read(chars, 0, 2048)) != -1) {
                if (fw != null)
                    fw.write(chars);
                chars = new char[2048];
            }
        } catch (Exception e) {
            System.out.println("error in post request: " + e);
            e.printStackTrace();
        }

        // use finally to close the input stream
        finally {
            try {
                if (out != null) {
                    out.close();
                }
                if (in != null) {
                    in.close();
                }
                if (fw != null) {
                    fw.close();
                }
            } catch (IOException ex) {
                ex.printStackTrace();
            }
        }
        return;
    }

    public String check(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=check";
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"check\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String check() {
        String res = this.check("GET");
        return res;
    }


    public String login(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=login&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"login\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String login() {
        String res = this.login("GET");
        return res;
    }

    public String testConnect(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=testConnect&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"testConnect\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String testConnect() {
        String res = this.testConnect("GET");
        return res;
    }


    public String getCoreVersion(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=getCoreVersion&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"getCoreVersion\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String getCoreVersion() {
        String res = this.getCoreVersion("GET");
        return res;
    }


    public String ipmanage(String type, String ip_type, String ips, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=ipmanage&type=" + type + "&ip_type=" + ip_type + "&ips=" + ips + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"ipmanage\",\"type\":\"" + type + "\",\"ip_type\":\"" + ip_type + "\", \"ips\":\"" + ips + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String ipmanage(String type, String ip_type, String ips) {
        String res = this.ipmanage(type, ip_type, ips, "GET");
        return res;
    }

    public String upload(String filePath) throws IOException {
        File fileToUpload = new File(filePath);
        if (!fileToUpload.exists() || !fileToUpload.isFile()) {
            throw new IOException("File is not exist: " + filePath);
        }
        String targetUrl = this.fileUrl + "/upload";
        URL url = new URL(targetUrl);
        HttpURLConnection httpConn = (HttpURLConnection) url.openConnection();
        try {
            String characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz123456789";
            StringBuilder sb = new StringBuilder();
            sb.append("----");
            Random random = new Random();
            for (int i = 0; i < 32; i++) {
                int index = random.nextInt(characters.length());
                sb.append(characters.charAt(index));
            }
            String boundary = sb.toString();
            // set method and content-type
            httpConn.setDoOutput(true);
            httpConn.setRequestMethod("POST");
            httpConn.setRequestProperty("Content-Type", "multipart/form-data; boundary=" + boundary);

            try (OutputStream outputStream = httpConn.getOutputStream();
                 FileInputStream inputStream = new FileInputStream(fileToUpload)) {
                // write params
                String parameterData = "--" + boundary + "\r\n" +
                        "Content-Disposition: form-data; name=\"username\"\r\n\r\n" +
                        this.username + "\r\n";
                outputStream.write(parameterData.getBytes(StandardCharsets.UTF_8));

                parameterData = "--" + boundary + "\r\n" +
                        "Content-Disposition: form-data; name=\"password\"\r\n\r\n" +
                        this.password + "\r\n";
                outputStream.write(parameterData.getBytes(StandardCharsets.UTF_8));

                // write file bytes
                String fileHeader = "--" + boundary + "\r\n" +
                        "Content-Disposition: form-data; name=\"file\"; filename=\"" + fileToUpload.getName() + "\"\r\n" +
                        "Content-Type: application/octet-stream\r\n\r\n";
                outputStream.write(fileHeader.getBytes(StandardCharsets.UTF_8));

                byte[] buffer = new byte[4096];
                int bytesRead;
                while ((bytesRead = inputStream.read(buffer)) != -1) {
                    outputStream.write(buffer, 0, bytesRead);
                }
                // write end boundary
                String boundaryEnd = "\r\n--" + boundary + "--\r\n";
                outputStream.write(boundaryEnd.getBytes(StandardCharsets.UTF_8));
            }

            int responseCode = httpConn.getResponseCode();
            if (responseCode == HttpURLConnection.HTTP_OK) {
                InputStream responseStream = httpConn.getInputStream();
                try (BufferedReader reader = new BufferedReader(new InputStreamReader(responseStream))) {
                    StringBuilder stringBuilder = new StringBuilder();
                    String line;
                    while ((line = reader.readLine()) != null) {
                        stringBuilder.append(line);
                    }
                    return stringBuilder.toString();
                }
            } else {
                throw new IOException("Response error: " + responseCode);
            }
        } finally {
            httpConn.disconnect();
        }
    }

    public String download(String filePath, String savePath) throws IOException {
        String fileURL = this.fileUrl + "/download";
        URL url = new URL(fileURL);
        HttpURLConnection httpConn = (HttpURLConnection) url.openConnection();
        try {
            String strPost = "filepath="+filePath+"&username=" + this.username + "&password=" + this.password;
            httpConn.setRequestMethod("POST");
            httpConn.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
            httpConn.setRequestProperty("Accept", "*/*");
            httpConn.setRequestProperty("Accept-Encoding", "gzip");

            httpConn.setDoOutput(true);
            httpConn.setDoInput(true);

            OutputStream connOutputStream = httpConn.getOutputStream();
            connOutputStream.write(strPost.getBytes(StandardCharsets.UTF_8));
            connOutputStream.flush();

            int responseCode = httpConn.getResponseCode();
            if (responseCode == HttpURLConnection.HTTP_OK) {
                String fileName = "";
                String disposition = httpConn.getHeaderField("Content-Disposition");
                if (disposition != null) {
                    // parse filename from content-disposition
                    int index = disposition.indexOf("filename=");
                    if (index > 0) {
                        fileName = disposition.substring(index + 10, disposition.length() - 1);
                    }
                } else {
                    // from filepath
                    fileName = filePath.substring(filePath.lastIndexOf("/") + 1);
                }

                // write to file
                String saveFilePath = savePath + File.separator + fileName;
                try (BufferedInputStream inputStream = new BufferedInputStream(httpConn.getInputStream());
                     FileOutputStream outputStream = new FileOutputStream(saveFilePath)) {

                    byte[] buffer = new byte[4096];
                    int bytesRead;
                    while ((bytesRead = inputStream.read(buffer)) != -1) {
                        outputStream.write(buffer, 0, bytesRead);
                    }
                }
                return saveFilePath;
            } else {
                throw new IOException("download failed: " + responseCode);
            }
        } finally {
            httpConn.disconnect();
        }
    }

    public String stat(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=stat&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"stat\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String stat() {
        String res = this.stat("GET");
        return res;
    }

    public String show(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=show&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"show\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String show() {
        String res = this.show("GET");
        return res;
    }


    public String load(String db_name, String csr, String request_type) {
        String res = "";
        if (csr == null || "".equals(csr)) {
            csr = "0";
        }
        if (request_type.equals("GET")) {
            String strUrl = "?operation=load&db_name=" + db_name + "&csr=" + csr + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"load\", \"db_name\": \"" + db_name + "\", \"csr\": \"" + csr + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String load(String db_name, String csr) {
        String res = this.load(db_name, csr, "GET");
        return res;
    }

    public String unload(String db_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=unload&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"unload\", \"db_name\": \"" + db_name + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String unload(String db_name) {
        String res = this.unload(db_name, "GET");
        return res;
    }


    public String monitor(String db_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=monitor&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"monitor\", \"db_name\": \"" + db_name + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String monitor(String db_name) {
        String res = this.monitor(db_name, "GET");
        return res;
    }

    public String build(String db_name, String db_path, String async, String callback, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=build&db_name=" + db_name + "&db_path=" + db_path + "&async=" + async + "&callback=" + callback + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"build\", \"db_name\": \"" + db_name + "\", \"db_path\": \"" + db_path + "\", \"async\": \"" + async + "\", \"callback\": \"" + callback + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String build(String db_name, String db_path, String async, String callback) {
        String res = this.build(db_name, db_path, async, callback, "GET");
        return res;
    }


    public String drop(String db_name, boolean is_backup, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl;
            if (is_backup) {
                strUrl = "?operation=drop&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password + "&is_backup=true";
            } else {
                strUrl = "?operation=drop&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password + "&is_backup=false";
            }
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost;
            if (is_backup) {
                strPost = "{\"operation\": \"drop\", \"db_name\": \"" + db_name + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"is_backup\": \"true\"}";
            } else {
                strPost = "{\"operation\": \"drop\", \"db_name\": \"" + db_name + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"is_backup\": \"false\"}";
            }
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String drop(String db_name, boolean is_backup) {
        String res = this.drop(db_name, is_backup, "GET");
        return res;
    }

    public String backup(String db_name, String backup_path, String async, String callback, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=backup&db_name=" + db_name + "&backup_path=" + backup_path + "&async=" + async + "&callback=" + callback + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"backup\", \"db_name\": \"" + db_name + "\", \"backup_path\": \"" + backup_path + "\", \"async\": \"" + async + "\", \"callback\": \"" + callback + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String backup(String db_name, String backup_path, String async, String callback) {
        String res = this.backup(db_name, backup_path, async, callback, "GET");
        return res;
    }


    public String backuppath(String db_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=backuppath&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"backuppath\", \"db_name\": \"" + db_name + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String backuppath(String db_name) {
        String res = this.backuppath(db_name, "GET");
        return res;
    }


    public String restore(String db_name, String backup_path, String async, String callback, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=restore&async=" + async + "&callback=" + callback + "&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&backup_path=" + backup_path;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"restore\",\"async\": \"" + async + "\",\"callback\": \"" + callback + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"backup_path\": \"" + backup_path + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String restore(String db_name, String backup_path, String async, String callback) {
        String res = this.restore(db_name, backup_path, async, callback, "GET");
        return res;
    }


    public String query(String db_name, String format, String sparql, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=query&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&format=" + format + "&sparql=" + sparql;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"query\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"format\": \"" + format + "\", \"sparql\": \"" + sparql + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String query(String db_name, String format, String sparql) {
        String res = this.query(db_name, format, sparql, "GET");
        return res;
    }

    public String exportDB(String db_name, String db_path, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=export&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&db_path=" + db_path;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"export\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"db_path\": \"" + db_path + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String exportDB(String db_name, String db_path) {
        String res = this.exportDB(db_name, db_path, "GET");
        return res;
    }


    public String batchInsert(String db_name, String file, String dir, String async, String callback, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=batchInsert&dir=" + dir + "&async=" + async + "&callback=" + callback + "&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&file=" + file;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"batchInsert\",\"dir\": \"" + dir + "\",\"async\": \"" + async + "\",\"callback\": \"" + callback + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"file\": \"" + file + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String batchInsert(String db_name, String file, String dir, String async, String callback) {
        String res = this.batchInsert(db_name, file, dir, async, callback, "GET");
        return res;
    }


    public String batchRemove(String db_name, String file, String async, String callback, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=batchRemove&async=" + async + "&callback=" + callback + "&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&file=" + file;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"batchRemove\",\"async\": \"" + async + "\",\"callback\": \"" + callback + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"file\": \"" + file + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String batchRemove(String db_name, String file, String async, String callback) {
        String res = this.batchRemove(db_name, file, async, callback, "GET");
        return res;
    }


    public String rename(String db_name, String new_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=rename&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&new_name=" + new_name;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"rename\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"new_name\": \"" + new_name + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String rename(String db_name, String new_name) {
        String res = this.rename(db_name, new_name, "GET");
        return res;
    }


    public String checkOperationState(String opt_id, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=checkOperationState&opt_id=" + opt_id + "&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"checkOperationState\", \"opt_id\": \"" + opt_id + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String checkOperationState(String opt_id) {
        String res = this.checkOperationState(opt_id, "GET");
        return res;
    }


    public String begin(String db_name, String isolevel, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=begin&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&isolevel=" + isolevel;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"begin\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"isolevel\": \"" + isolevel + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String begin(String db_name, String isolevel) {
        String res = this.begin(db_name, isolevel, "GET");
        return res;
    }


    public String tquery(String db_name, String tid, String sparql, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=tquery&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&tid=" + tid + "&sparql=" + sparql;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"tquery\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"tid\": \"" + tid + "\", \"sparql\": \"" + sparql + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String tquery(String db_name, String tid, String sparql) {
        String res = this.tquery(db_name, tid, sparql, "GET");
        return res;
    }


    public String commit(String db_name, String tid, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=commit&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&tid=" + tid;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"commit\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"tid\": \"" + tid + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String commit(String db_name, String tid) {
        String res = this.commit(db_name, tid, "GET");
        return res;
    }

    public String rollback(String db_name, String tid, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=rollback&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&tid=" + tid;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"rollback\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"tid\": \"" + tid + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String rollback(String db_name, String tid) {
        String res = this.rollback(db_name, tid, "GET");
        return res;
    }


    public String checkpoint(String db_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=checkpoint&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"checkpoint\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String checkpoint(String db_name) {
        String res = this.checkpoint(db_name, "GET");
        return res;
    }


    public String showuser(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=showuser&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"showuser\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String showuser() {
        String res = this.showuser("GET");
        return res;
    }

    public String usermanage(String type, String op_username, String op_password, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=usermanage&type=" + type + "&username=" + this.username + "&password=" + this.password + "&op_username=" + op_username + "&op_password=" + op_password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"usermanage\", \"type\": \"" + type + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"op_username\": \"" + op_username + "\", \"op_password\": \"" + op_password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String usermanage(String type, String op_username, String op_password) {
        String res = this.usermanage(type, op_username, op_password, "GET");
        return res;
    }


    public String userprivilegemanage(String type, String op_username, String privileges, String db_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=userprivilegemanage&type=" + type + "&username=" + this.username + "&password=" + this.password + "&op_username=" + op_username + "&privileges=" + privileges + "&db_name=" + db_name;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"userprivilegemanage\", \"type\": \"" + type + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"op_username\": \"" + op_username + "\", \"privileges\": \"" + privileges + "\", \"db_name\": \"" + db_name + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String userprivilegemanage(String type, String op_username, String privileges, String db_name) {
        String res = this.userprivilegemanage(type, op_username, privileges, db_name, "GET");
        return res;
    }


    public String userpassword(String re_username, String re_password, String op_password, String db_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=userpassword&re_username=" + re_username + "&username=" + this.username + "&password=" + this.password + "&re_password=" + re_password + "&op_password=" + op_password + "&db_name=" + db_name;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"userpassword\", \"re_username\": \"" + re_username + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"re_password\": \"" + re_password + "\", \"op_password\": \"" + op_password + "\", \"db_name\": \"" + db_name + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String userpassword(String re_username, String re_password, String op_password, String db_name) {
        String res = this.userpassword(re_username, re_password, op_password, db_name, "GET");
        return res;
    }


    public String funquery(String funInfo, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=funquery&username=" + this.username + "&password=" + this.password + "&funInfo=" + funInfo;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"funquery\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"funInfo\": \"" + funInfo + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String funquery(String funInfo) {
        String res = this.funquery(funInfo, "GET");
        return res;
    }

    public String funcudb(String type, String funInfo, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=funcudb&type=" + type + "&username=" + this.username + "&password=" + this.password + "&funInfo=" + funInfo;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"funcudb\",\"type\": \"" + type + "\",  \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"funInfo\": \"" + funInfo + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String funcudb(String type, String funInfo) {
        String res = this.funcudb(type, funInfo, "GET");
        return res;
    }


    public String funreview(String funInfo, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=funreview&username=" + this.username + "&password=" + this.password + "&funInfo=" + funInfo;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"funreview\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"funInfo\": \"" + funInfo + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String funreview(String funInfo) {
        String res = this.funreview(funInfo, "GET");
        return res;
    }

    public String txnlog(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=txnlog&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"txnlog\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String txnlog() {
        String res = this.txnlog("GET");
        return res;
    }


    public String querylogdate(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=querylogdate&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"querylogdate\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String querylogdate() {
        String res = this.querylogdate("GET");
        return res;
    }


    public String querylog(String date, String pageNo, String pageSize, String db_name, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=querylog&date=" + date + "&username=" + this.username + "&password=" + this.password + "&pageNo=" + pageNo + "&pageSize=" + pageSize + "&db_name=" + db_name;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"querylog\", \"date\": \"" + date + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"pageNo\": \"" + pageNo + "\", \"pageSize\": \"" + pageSize + "\", \"db_name\": \"" + db_name + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String querylog(String date, String pageNo, String pageSize, String db_name) {
        String res = this.querylog(date, pageNo, pageSize, db_name, "GET");
        return res;
    }

    public String accesslogdate(String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=accesslogdate&username=" + this.username + "&password=" + this.password;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"accesslogdate\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String accesslogdate() {
        String res = this.accesslogdate("GET");
        return res;
    }


    public String accesslog(String date, String pageNo, String pageSize, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=accesslog&date=" + date + "&username=" + this.username + "&password=" + this.password + "&pageNo=" + pageNo + "&pageSize=" + pageSize;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"accesslog\", \"date\": \"" + date + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"pageNo\": \"" + pageNo + "\", \"pageSize\": \"" + pageSize + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String accesslog(String date, String pageNo, String pageSize) {
        String res = this.accesslog(date, pageNo, pageSize, "GET");
        return res;
    }

    public String addReason(String type, String db_name, String ruleinfo, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=reasonManage&type=" + type + "&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&ruleinfo=" + ruleinfo;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"reasonManage\", \"type\": \"" + type + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"ruleinfo\": \"" + ruleinfo + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String addReason(String type, String db_name, String ruleinfo) {
        String res = this.addReason(type, db_name, ruleinfo, "GET");
        return res;
    }

    public String reasonManage(String type, String db_name, String rulename, String request_type) {
        String res = "";
        if (request_type.equals("GET")) {
            String strUrl = "?operation=reasonManage&type=" + type + "&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&rulename=" + rulename;
            res = this.sendGet(strUrl);
        } else if (request_type.equals("POST")) {
            String strPost = "{\"operation\": \"reasonManage\", \"type\": \"" + type + "\", \"username\": \"" + this.username + "\", \"password\": \"" + this.password + "\", \"db_name\": \"" + db_name + "\", \"rulename\": \"" + rulename + "\"}";
            res = this.sendPost(strPost);
        }
        return res;
    }

    public String reasonManage(String type, String db_name, String rulename) {
        String res = this.reasonManage(type, db_name, rulename, "GET");
        return res;
    }
}
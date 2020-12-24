package jgsc;

import java.io.*;
import java.net.*;

public class GstoreConnector {

    public static final String defaultServerIP = "127.0.0.1";
    public static final int defaultServerPort = 3305;

    private String serverIP;
    private int serverPort;
    private Socket socket = null;

    public GstoreConnector() {
        this.serverIP = GstoreConnector.defaultServerIP;
        this.serverPort = GstoreConnector.defaultServerPort;
    }

    public GstoreConnector(int _port) {
        this.serverIP = GstoreConnector.defaultServerIP;
        this.serverPort = _port;
    }

    public GstoreConnector(String _ip, int _port) {
        this.serverIP = _ip;
        this.serverPort = _port;
    }

    public boolean test() {
        boolean connect_return = this.connect();
        if (!connect_return) {
            System.err.println("connect to server error. @GstoreConnector.test");
            return false;
        }

        String cmd = "test";
        boolean send_return = this.send(cmd);
        if (!send_return) {
            System.err.println("send test command error. @GstoreConnector.test");
            return false;
        }
        String recv_msg = this.recv();

        this.disconnect();
        System.out.println(recv_msg);

        return recv_msg.equals("OK");
    }

    public boolean load(String _db_name) {
        boolean connect_return = this.connect();
        if (!connect_return) {
            System.err.println("connect to server error. @GstoreConnector.load");
            return false;
        }

        String cmd = "load " + _db_name;
        boolean send_return = this.send(cmd);
        if (!send_return) {
            System.err.println("send load command error. @GstoreConnector.load");
            return false;
        }
        String recv_msg = this.recv();

        this.disconnect();

        System.out.println(recv_msg);
        if (recv_msg.equals("load database done.")) {
            return true;
        }

        return false;
    }

    public boolean unload(String _db_name) {
        boolean connect_return = this.connect();
        if (!connect_return) {
            System.err.println("connect to server error. @GstoreConnector.unload");
            return false;
        }

        String cmd = "unload " + _db_name;
        boolean send_return = this.send(cmd);
        if (!send_return) {
            System.err.println("send unload command error. @GstoreConnector.unload");
            return false;
        }
        String recv_msg = this.recv();

        this.disconnect();

        System.out.println(recv_msg);
        if (recv_msg.equals("unload database done.")) {
            return true;
        }

        return false;
    }

    public boolean build(String _db_name, String _rdf_file_path) {
        boolean connect_return = this.connect();
        if (!connect_return) {
            System.err.println("connect to server error. @GstoreConnector.build");
            return false;
        }

        String cmd = "import " + _db_name + " " + _rdf_file_path;
        boolean send_return = this.send(cmd);
        if (!send_return) {
            System.err.println("send import command error. @GstoreConnector.build");
            return false;
        }
        String recv_msg = this.recv();

        this.disconnect();

        System.out.println(recv_msg);
        if (recv_msg.equals("import RDF file to database done.")) {
            return true;
        }

        return false;
    }

    public boolean drop(String _db_name) {
        boolean connect_return = this.connect();
        if (!connect_return) {
            System.err.println("connect to server error. @GstoreConnector.drop");
            return false;
        }

        String cmd = "drop " + _db_name;
        boolean send_return = this.send(cmd);
        if (!send_return) {
            System.err.println("send drop command error. @GstoreConnector.drop");
            return false;
        }
        String recv_msg = this.recv();

        this.disconnect();

        System.out.println(recv_msg);
        return recv_msg.equals("drop database done.");
    }

    public String query(String _sparql) {
        return this.query(_sparql, "/");
    }

    public String query(String _sparql, String _output) {
        boolean connect_return = this.connect();
        if (!connect_return) {
            System.err.println("connect to server error. @GstoreConnector.query");
            return "connect to server error.";
        }

        String cmd = "query " + _sparql;
        boolean send_return = this.send(cmd);
        if (!send_return) {
            System.err.println("send query command error. @GstoreConnector.query");
            return "send query command error.";
        }
        String recv_msg = this.recv();

        this.disconnect();

        return recv_msg;
    }

    public String show() {
        return this.show(false);
    }

    public String show(boolean _type) {
        boolean connect_return = this.connect();
        if (!connect_return) {
            System.err.println("connect to server error. @GstoreConnector.show");
            return "connect to server error.";
        }

        String cmd;
        if (_type) {
            cmd = "show all";
        } else {
            cmd = "show databases";
        }
        boolean send_return = this.send(cmd);
        if (!send_return) {
            System.err.println("send show command error. @GstoreConnector.show");
            return "send show command error.";
        }
        String recv_msg = this.recv();

        this.disconnect();
        return recv_msg;
    }

    private boolean connect() {
        try {
            this.socket = new Socket(this.serverIP, this.serverPort);
            return (this.socket != null && this.socket.isConnected() && this.socket.isBound());
        } catch (Exception e) {
            System.err.println("socket connection error. @GstoreConnector.connect");
            e.printStackTrace();
        }

        return false;
    }

    private boolean disconnect() {
        try {
            if (this.socket != null) {
                this.socket.close();
                this.socket = null;
            }

            return true;
        } catch (IOException e) {
            System.err.println("socket disconnection error. @GstoreConnector.disconnect");
            e.printStackTrace();
        }

        return false;
    }

    private boolean send(String _msg) {
        byte[] data = GstoreConnector.packageMsgData(_msg);

        try {
            DataOutputStream dos = new DataOutputStream(this.socket.getOutputStream());
            dos.write(data);

            return true;
        } catch (IOException e) {
            System.err.println("send message error. @GstoreConnector.send");
            e.printStackTrace();
        }

        return false;
    }

    private String recv() {
        try {
            DataInputStream dis = new DataInputStream(this.socket.getInputStream());
            byte[] head = new byte[4];
            dis.read(head, 0, 4);
            int context_len = GstoreConnector.byte4ToInt(head);

            // in Java String, there is no need for terminator '\0' in C. so we should omit '\0' at the end of receiving message.
            byte[] data_context = new byte[context_len - 1];
            int recv_len = 0;
            do {
                int cur_len = dis.read(data_context, recv_len, data_context.length - recv_len);
                recv_len += cur_len;
            } while (recv_len < data_context.length);

            String ret = new String(data_context, "utf-8");

            return ret;
        } catch (IOException e) {
            System.err.println("receive message error. @GstoreConnector.send");
            e.printStackTrace();
        }

        return "receive message from server error.";
    }

    private static byte[] packageMsgData(String _msg) {
        //byte[] data_context = _msg.getBytes();
        byte[] data_context = null;
        try {
            data_context = _msg.getBytes("utf-8");
        } catch (UnsupportedEncodingException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            System.err.println("utf-8 charset is unsupported.");
            data_context = _msg.getBytes();
        }
        int context_len = data_context.length + 1; // 1 byte for '\0' at the end of the context.
        int data_len = context_len + 4; // 4 byte for one int(data_len at the data's head).
        byte[] data = new byte[data_len];

        // padding head(context_len).
        byte[] head = GstoreConnector.intToByte4(context_len);
        for (int i = 0; i < 4; i++) {
            data[i] = head[i];
        }

        // padding context.
        for (int i = 0; i < data_context.length; i++) {
            data[i + 4] = data_context[i];
        }
        // in C, there should be '\0' as the terminator at the end of a char array. so we need add '\0' at the end of sending message.
        data[data_len - 1] = 0;

        return data;
    }

    private static byte[] intToByte4(int _x) { // with Little Endian format.
        byte[] ret = new byte[4];
        ret[0] = (byte) (_x);
        ret[1] = (byte) (_x >>> 8);
        ret[2] = (byte) (_x >>> 16);
        ret[3] = (byte) (_x >>> 24);

        return ret;
    }

    private static int byte4ToInt(byte[] _b) { // with Little Endian format.
        int byte0 = _b[0] & 0xFF, byte1 = _b[1] & 0xFF, byte2 = _b[2] & 0xFF, byte3 = _b[3] & 0xFF;
        int ret = (byte0) | (byte1 << 8) | (byte2 << 16) | (byte3 << 24);

        return ret;
    }

    public static void main(String[] args) {
        // initialize the GStore server's IP address and port.
        GstoreConnector gc = new GstoreConnector("172.31.19.15", 3305);

        // build a new database by a RDF file.
        // note that the relative path is related to gserver.
        //gc.build("db_LUBM10", "example/rdf_triple/LUBM_10_GStore.n3");
        String sparql = "select ?x where {"
                        + "?x	<rdf:type>	<cdblp.cn/class/Paper>. "
                        + "?x	<cdblp.cn/schema/property/has_author>	<cdblp.cn/author/wangshan>. "
                        + "}";

        boolean flag = gc.load("db_cdblp");
        System.out.println(flag);
        String answer = gc.query(sparql);
        System.out.println(answer);

        answer = gc.query(sparql);
        System.out.println(answer);

        sparql = "select ?x where {"
                 + "?x	<rdf:type>	<cdblp.cn/class/Paper>. "
                 + "?x	<cdblp.cn/schema/property/has_author>	<cdblp.cn/author/yuge>. "
                 + "}";
        answer = gc.query(sparql);
        System.out.println(answer);
    }
}

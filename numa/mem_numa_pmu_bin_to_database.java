
/*
 *  Filename: Data2DB.java
 *  excution example:
 *  /opt/jdk/jdk1.7.0_55/bin/java -Dfile.encoding=ANSI_X3.4-1968 -classpath /home/shine/workspace/PMUDataPaser/bin:/home/shine/javalib/mysql-connector-java-5.1.39-bin.jar Data2DB /tmp/pmu-data/pmu_count.bin.SPECRUN-0921-nopin-mon-1.1474602311 1
 *
 * putput example:
 * Use File: /tmp/pmu-data/pmu_count.bin.SPECRUN-0921-pin-mon-6.1474611662
 * DB Operation: True
 * Total records in the file: 2435, record size 3080, file size 7499832.
 * 123456789ABCDEF, 48, 8, 998, 10800.
 * 81985529216486895, 48, 8, 998, 10800.
 * Connecting to database...
 * ===> Fri Sep 23 12:42:45 CST 2016 (1474605765)
 * ===> Fri Sep 23 12:42:46 CST 2016 (1474605766)
 * ===> Fri Sep 23 12:42:47 CST 2016 (1474605767)
 * ===> Fri Sep 23 12:42:48 CST 2016 (1474605768)
 * ===> Fri Sep 23 12:42:49 CST 2016 (1474605769)
 * ===> Fri Sep 23 12:42:50 CST 2016 (1474605770)
 * ===> Fri Sep 23 12:42:51 CST 2016 (1474605771)
 * ...
 *
 */

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.util.Date;
import java.sql.*;

public class Data2DB {

        // JDBC driver name and database URL
        static final String JDBC_DRIVER = "com.mysql.jdbc.Driver";
        static final String DB_URL = "jdbc:mysql://10.100.17.203/pmu";
        static final long binFileHeaderLen = 32L;

        // Database credentials
        static final String USER = "root";
        static final String PASS = "******";

        boolean dbOps = false;

        public Connection openDB() {
                Connection conn = null;
                try {
                        // STEP 2: Register JDBC driver
                        Class.forName("com.mysql.jdbc.Driver");

                        // STEP 3: Open a connection
                        System.out.println("Connecting to database...");
                        conn = DriverManager.getConnection(DB_URL, USER, PASS);

                } catch (SQLException se) {
                        // Handle errors for JDBC
                        se.printStackTrace();
                } catch (Exception e) {
                        // Handle errors for Class.forName
                        e.printStackTrace();
                }
                return conn;
        }

        public boolean closeDB(Connection conn) {

                try {
                        conn.close();

                } catch (SQLException se) {
                        // Handle errors for JDBC
                        se.printStackTrace();
                } catch (Exception e) {
                        // Handle errors for Class.forName
                        e.printStackTrace();
                }
                return true;
        }

        public boolean insert(Connection conn, long ts, int cpu, int evt, long val) {

                PreparedStatement stmt = null;
                boolean action = false;
                try {
                        String sql;
                        sql = "INSERT INTO memtest VALUES (?, ?, ?, ?)";
                        stmt = conn.prepareStatement(sql);
                        java.util.Date now = new java.util.Date();
                        stmt.setLong(1, ts);
                        stmt.setInt(2, cpu);
                        stmt.setInt(3, evt);
                        stmt.setLong(4, val);
                        action = stmt.execute();
                        stmt.close();

                } catch (SQLException se) {
                        // Handle errors for JDBC
                        se.printStackTrace();
                } catch (Exception e) {
                        // Handle errors for Class.forName
                        e.printStackTrace();
                } finally {
                        // finally block used to close resources
                        try {
                                if (stmt != null)
                                        stmt.close();
                        } catch (SQLException se2) {
                        } // nothing we can do

                } // end try
                return action;
        }

        public boolean filePaser(String filePath) {
                // String encoding = "UTF-8";
                long magic;
                int nr_cpu;
                int nr_event;
                long delay;
                long repeat, actual_records;
                long time_stamp;
                long val;
                long fileLen = 0L;
                Connection conn = null;
                try {
                        File file = new File(filePath);
                        DataInputStream in = null;
                        fileLen = file.length();
                        if (file.isFile() && file.exists() && fileLen > binFileHeaderLen) {
                                in = new DataInputStream(new FileInputStream(file));
                        } else {
                                return false;
                        }
                        magic = Long.reverseBytes(in.readLong());
                        nr_cpu = Integer.reverseBytes(in.readInt());
                        nr_event = Integer.reverseBytes(in.readInt());
                        delay = Long.reverseBytes(in.readLong());
                        repeat = Long.reverseBytes(in.readLong());

                        // size checking
                        actual_records = (fileLen - binFileHeaderLen) / (8 + 8 * nr_cpu * nr_event);
                        System.out.printf("Total records in the file: %d, record size %d, file size %d.\n",
                                        actual_records, 8 + 8 * nr_cpu * nr_event, fileLen);
                        System.out.printf("%X, %d, %d, %d, %d.\n", magic, nr_cpu, nr_event, delay, repeat);
                        System.out.println(
                                        magic + ", " + nr_cpu + ", " + nr_event + ", " + delay + ", " + repeat + ".");

                        if (dbOps) {
                                conn = openDB();
                        }
                        while (actual_records-- > 0) {
                                time_stamp = Long.reverseBytes(in.readLong());
                                Date expiry = new Date(time_stamp * 1000L);
                                System.out.println("===> " + expiry + " (" + time_stamp +  ")");
                                int cpu_loop = 0;
                                while (cpu_loop++ < nr_cpu) {
                                        int evt_loop = 0;
                                        if (!dbOps) {
                                                System.out.print(cpu_loop + ":\t");
                                        }

                                        while (evt_loop++ < nr_event) {
                                                val = Long.reverseBytes(in.readLong());

                                                if (dbOps) {
                                                        insert(conn, time_stamp, cpu_loop, evt_loop, val);
                                                } else {
                                                        System.out.print(val + "\t");
                                                }
                                        }
                                        if (!dbOps) {
                                                System.out.println("");
                                        }
                                }
                        }
                        if (dbOps) {
                                conn.close();
                        }
                        in.close();
                } catch (NullPointerException e) {
                        if (conn == null) {
                                System.out.println("Connection lost?");
                        }
                } catch (Exception e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                } finally {
                        try {
                                if (conn != null)
                                        conn.close();
                        } catch (SQLException se) {
                                se.printStackTrace();
                        } // end finally try
                } // end try

                return true;
        }

        public static void main(String[] args) {
                Data2DB e = new Data2DB();
                String fileName = "/tmp/pmu_count.bin";

                if (args.length > 0) {
                        fileName = args[0];
                        System.out.println("Use File: " + fileName);
                }
                if (args.length > 1 && (Integer.parseInt(args[1]) == 1)) {
                        System.out.println("DB Operation: True");
                        e.dbOps = true;
                }

                e.filePaser(fileName);

        }

}


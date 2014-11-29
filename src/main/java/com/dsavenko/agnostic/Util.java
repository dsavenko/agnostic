package com.dsavenko.agnostic;

import java.io.BufferedInputStream;
import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;

public class Util {

	private Util() {
	}

	public static String fixNull(String s) {
		return null == s ? "" : s;
	}

	public static int runProcess(File basedir, String... args) {
		ProcessBuilder pb = new ProcessBuilder(args);
		pb.directory(basedir);
		pb.redirectErrorStream(true);
		InputStream in = null;
		try {
			Process p = pb.start();
			in = new BufferedInputStream(p.getInputStream());
			byte[] b = new byte[1024];
			int c;
			while (-1 != (c = in.read(b))) {
				System.out.write(b, 0, c);
			}
			return p.waitFor();
		} catch (Exception e) {
			throw new AgnosticException(e.getMessage(), e);
		} finally {
			Util.safeClose(in);
		}
	}

	public static void safeClose(Closeable... toClose) {
		// doesn't have newer version of IOUtils with closeQuietly(Closeable)
		for (Closeable f : toClose) {
			if (null != f) {
				try {
					f.close();
				} catch (IOException e) {
					// ignore
				}
			}
		}
	}

}

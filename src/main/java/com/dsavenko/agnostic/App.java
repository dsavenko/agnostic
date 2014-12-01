package com.dsavenko.agnostic;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;

public class App {

	private static final String DEFAULT_CONFIG_FILE = "agnostic.yaml";

	public static void main(String[] args) throws IOException {
		try {
			if (1 > args.length) {
				help();
			} else {
				String cmd = args[0];
				String[] restArgs = Arrays.copyOfRange(args, 1, args.length);
				runCmd(cmd, restArgs);
			}
		} catch (Throwable e) {
			e.printStackTrace();
		}
		System.exit(0);
	}

	private static void runCmd(String cmd, String[] args) {
		switch (cmd) {
			case "checkout":
				checkout();
				break;
			case "build-up":
				buildUp();
				break;
			case "build":
				build();
				break;
			case "run":
				runScript(args);
				break;
			default:
				help();
				throw new AgnosticException("Unknown command: " + cmd);
		}
	}

	private static File currentDir() {
		return new File(System.getProperty("user.dir"));
	}

	private static String currentProject() {
		return currentDir().getAbsoluteFile().getName();
	}

	private static File config() {
		File currentDir = currentDir();
		File parent = currentDir.getParentFile();
		File parentConfig = new File(parent, DEFAULT_CONFIG_FILE);
		if (!parentConfig.exists()) {
			throw new AgnosticException("Config file not found: " + parentConfig.getAbsolutePath());
		}
		return parentConfig;
	}

	private static Components components(File config) {
		return Components.load(config);
	}

	private static void checkout() {
		components(new File(DEFAULT_CONFIG_FILE)).checkoutAll();
	}

	private static void buildUp() {
		components(config()).buildUp(currentProject());
	}

	private static void build() {
		components(config()).build(currentProject());
	}

	private static void help() {
		System.out.println("java -jar agnostic.jar <command> [command] [command] ...\n" +
			"Commands: help, checkout, build, build-up, run");
	}

	private static void runScript(String[] args) {
		String name = args[0];
		String[] restArgs = Arrays.copyOfRange(args, 1, args.length);
		components(config()).runScript(currentProject(), name, restArgs);
	}

}

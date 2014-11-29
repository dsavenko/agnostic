package com.dsavenko.agnostic;

import org.apache.commons.io.FileUtils;

import java.io.File;
import java.io.IOException;
import java.util.Collections;
import java.util.List;
import java.util.Map;

public final class Project {

	private Map yaml;

	Project(Map yaml) {
		this.yaml = yaml;
	}

	public String getName() {
		return (String) yaml.get("name");
	}

	public String getDescription() {
		return (String) yaml.get("description");
	}

	public boolean isHg() {
		return null != getHg();
	}

	public String getHg() {
		return (String) yaml.get("hg");
	}

	public boolean isGit() {
		return null != getGit();
	}

	public String getGit() {
		return (String) yaml.get("git");
	}

	public String getBuild() {
		return (String) yaml.get("build");
	}

	public List<String> getBuildAfter() {
		List deps = (List) yaml.get("buildAfter");
		if (null == deps) {
			return Collections.emptyList();
		} else {
			return Collections.unmodifiableList(deps);
		}
	}

	private String[] vcsArgs() {
		if (isGit()) {
			return new String[]{"git", "clone", getGit(), getName()};
		}
		if (isHg()) {
			return new String[]{"hg", "clone", getHg(), getName()};
		}
		throw new AgnosticException("Unknown or unspecified VCS");
	}

	public void checkout(File parentDir) {
		System.out.println("Checkout started: " + getName());
		int ret = Util.runProcess(parentDir, vcsArgs());
		if (0 != ret) {
			throw new AgnosticException("Failed to checkout with ret code " + ret + ": " + getName());
		}
		System.out.println("Checkout finished: " + getName());
	}

	public void build(File parentDir) {
		System.out.println("Building started: " + getName());
		File script = null;
		File projectDir = new File(parentDir, getName());
		if (!projectDir.exists()) {
			throw new AgnosticException("Project directory doesn't exist: " + projectDir.getAbsolutePath());
		}
		try {
			script = File.createTempFile("agnostic-script-", "sh");
			FileUtils.writeStringToFile(script, getBuild());
			int ret = Util.runProcess(projectDir, "/bin/sh", "-xe", script.getAbsolutePath());
			if (0 != ret) {
				throw new AgnosticException("Failed to build, ret code: " + ret);
			}
			System.out.println("Building finished: " + getName());
		} catch (IOException e) {
			throw new AgnosticException(e.getMessage(), e);
		} finally {
			if (null != script) {
				script.delete();
			}
		}
	}

	@Override
	public boolean equals(Object o) {
		if (this == o) {
			return true;
		}
		if (!(o instanceof Project)) {
			return false;
		}

		Project project = (Project) o;

		final String thisName = getName();
		final String thatName = project.getName();

		if (thisName != null ? !thisName.equals(thatName) : thatName != null) {
			return false;
		}

		return true;
	}

	@Override
	public int hashCode() {
		final String name = getName();
		return name != null ? name.hashCode() : 0;
	}
}

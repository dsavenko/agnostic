package com.dsavenko.agnostic;

import org.yaml.snakeyaml.Yaml;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

public class Components {

	private final ScheduledExecutorService execService = Executors.newScheduledThreadPool(5);

	private List<Project> projects = new ArrayList<>();
	private final File parent;

	public static Components load(File agnosticYaml) {
		try {
			try (InputStream in = new FileInputStream(agnosticYaml)) {
				return new Components(agnosticYaml.getAbsoluteFile().getParentFile(), new Yaml().loadAll(in));
			}
		} catch (IOException e) {
			throw new AgnosticException(e.getMessage(), e);
		}
	}

	private Components(File parent, Iterable<Object> components) {
		this.parent = parent;
		for (Object o : components) {
			Map m = (Map) o;
			projects.add(new Project((Map) m.get("project")));
		}
	}

	public List<Project> getProjects() {
		return Collections.unmodifiableList(projects);
	}

	public void checkoutAll() {
		checkoutList(parent, getProjects());
	}

	private void checkoutList(final File parentDir, List<Project> projects) {
		List<ScheduledFuture<?>> futures = new ArrayList<>();
		for (final Project p : projects) {
			ScheduledFuture<?> f = execService.schedule(new Runnable() {
				@Override
				public void run() {
					p.checkout(parentDir);
				}
			}, 0, TimeUnit.SECONDS);
			futures.add(f);
		}
		for (ScheduledFuture<?> f : futures) {
			try {
				f.get();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	public Project getProject(String projectName) {
		for (Project p : getProjects()) {
			if (p.getName().equals(projectName)) {
				return p;
			}
		}
		return null;
	}

	public void buildUp(String projectName) {
		for (Project p : preparePredecessorsList(projectName)) {
			p.build(parent);
		}
	}

	public void build(String projectName) {
		Project project = getProject(projectName);
		if (null == project) {
			throw new AgnosticException("Project not found: " + projectName);
		}
		project.build(parent);
	}

	private List<Project> preparePredecessorsList(String projectName) {
		Project project = getProject(projectName);
		if (null == project) {
			throw new AgnosticException("Project not found: " + projectName);
		}
		List<Project> ret = new ArrayList<>();
		fillPredecessorsList(project, ret);
		return removeDuplicatesAndReverse(ret);
	}

	private void fillPredecessorsList(Project project, List<Project> depList) {
		depList.add(project);
		for (String depName : project.getBuildAfter()) {
			Project dep = getProject(depName);
			if (null == dep) {
				throw new AgnosticException("Dependency for " + project.getName() + " not found: " + depName);
			}
			fillPredecessorsList(dep, depList);
		}
	}

	private List<Project> removeDuplicatesAndReverse(List<Project> depList) {
		Set<String> names = new HashSet<>();
		List<Project> ret = new ArrayList<>();
		for (int i = depList.size() - 1; 0 <= i; --i) {
			Project p = depList.get(i);
			if (!names.contains(p.getName())) {
				ret.add(p);
				names.add(p.getName());
			}
		}
		return ret;
	}

}

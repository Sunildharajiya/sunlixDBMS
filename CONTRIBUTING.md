# Contributing to SunlixDBMS

Thank you for contributing to SunlixDBMS! 

Please follow these guidelines to keep the project organized, maintainable, and reliable.

---

##  General Rules

- Do not work directly on the `main` branch
- Every contribution must be made through a Pull Request
- Keep each branch focused on one feature, issue, or improvement
- Do not include unrelated changes in a single Pull Request
- Follow the existing project structure and coding style
- Write clear and meaningful commit messages
- Ensure the project builds successfully before requesting a merge
- Do not commit:
  - Generated files
  - Local database files
  - Secrets or API keys
  - Unnecessary files

---

##  Adding a New Feature

Follow these steps to add a new feature:

1. Fork the repository
2. Create a new branch specifically for the feature
3. Implement the feature
4. Add tests for the new functionality
5. Add or update the related documentation
6. Open a Pull Request to the `main` branch
7. Address review feedback using additional commits on the same branch
8. The feature can be merged only after tests and documentation are complete

###  Testing Requirement

Every new feature must include corresponding test(s) before final merge:

- Tests should verify the important behavior of the functionality
- Include relevant edge cases and failure conditions
- No feature is considered complete without appropriate tests

---

##  Fixing an Issue

Follow these steps to fix an existing GitHub issue:

1. Fork the repository or update your existing fork
2. Create a new branch using the format: `<issue-number>-<short-issue-name>`
   - Example: `27-json-reader`
3. Reproduce and understand the issue
4. Implement the fix
5. Add or update a regression test for the issue
6. Update documentation if the fix changes documented behavior
7. Open a Pull Request targeting `main`
8. Reference the issue using: `Closes #<issue-number>`
   - Example: `Closes #27`

---

##  Pull Request Updates

- Continue using the same branch for changes requested during review
- Do not create a new Pull Request for every review change
- Push additional commits to the existing branch so they automatically appear in the same Pull Request

---

##  Documentation

### Placement

All new project documentation must be placed in the `documentation/` directory.

If documentation for the relevant functionality already exists, update the existing file instead of creating duplicates.

### When to Update Documentation

Update documentation whenever a contribution introduces or changes:

- Features
- Functions or APIs
- CLI commands
- Configuration
- User-facing behavior
- Project architecture
- Important implementation details

---

##  Tests

Tests are required for all new functionality.

### Before Final Merge

- New functionality must have a corresponding test file
- Bug fixes should include a regression test
- Existing tests must continue to pass
- New tests must pass
- Important edge cases should be covered

**Code without the required tests is not ready for final merge.**

---

##  Branch Naming

### New Features

Use a clear, descriptive name for the feature:

- Format: `<feature-name>`
- Examples:
  - `json-reader`
  - `key-generator`
  - `cli-improvements`

### Issue Fixes

Always include the issue number:

- Format: `<issue-number>-<short-issue-name>`
- Examples:
  - `27-json-reader`
  - `34-cli-crash`
  - `41-writer-error`

---

##  Pull Request Requirements

Every Pull Request should:

- Have a clear, descriptive title
- Clearly explain what was changed
- Explain why the change was needed
- Include relevant tests
- Include documentation when required
- Reference the related issue when applicable (use `Closes #<issue-number>`)
- Contain only relevant changes
- Be ready for code review

---

##  Pre-Merge Checklist

Before a contribution can be merged, ensure all applicable requirements are met:

- [ ] Code is implemented correctly
- [ ] Project builds successfully
- [ ] Required tests are added
- [ ] All tests pass
- [ ] Documentation is added or updated in `documentation/`
- [ ] No unrelated changes are included
- [ ] Pull Request description is complete
- [ ] Review feedback has been addressed

---

##  Contribution Principle

Every contribution should follow this workflow:

```
Build → Test → Document → Review → Merge
```

Let's keep SunlixDBMS clean, reliable, and easy to maintain.

Thank you for your contributions! 

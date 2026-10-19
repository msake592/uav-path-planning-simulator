# AI Development Instructions

## Project
3D UAV Path Planning & Optimization Simulator

## Technology Stack
- C++20
- CMake
- GoogleTest
- React
- TypeScript
- Three.js

## Architecture
- The simulation core must remain independent of visualization.
- Planning algorithms must not depend on React or Three.js.
- Prefer modular, testable, and maintainable C++ code.
- Use RAII and modern C++ practices.
- Avoid unnecessary dependencies.

## Development Rules
- Read docs/PROJECT_PLAN.md before major changes.
- Do not introduce major architectural changes without discussion.
- Add tests for algorithmic changes.
- Explain important design decisions.
- Do not modify unrelated components.
- Do not implement future roadmap features without approval.

## Learning Objectives
This is an educational engineering project.

For algorithms:
- Explain mathematical foundations.
- State computational complexity.
- Document assumptions.
- Include relevant test cases.
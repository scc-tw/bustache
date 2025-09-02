# Roadmap: Mustache Spec v1.1.3 → v1.4.3

This project currently targets Mustache specification v1.4.3. The following roadmap documents the changes introduced after the older v1.1.3 baseline and notes their implementation status.

## v1.2.0
- [x] Introduced optional **Inheritance** module (`{{< parent}}` / `{{/parent}}`).
- [x] Added optional lambda tests for Raku.
- [x] Expanded tests for context stack behaviour.
- [x] Project relicensed under MIT.

## v1.2.1
- [x] Specified how to interpolate `null` values.
- [x] Clarified interpolation of implicit iterators (`{{.}}`).

## v1.2.2
- [x] Added PowerShell lambda fixtures.
- [x] Defined scope resolution for substituted blocks in inheritance.

## v1.3.0
- [x] **Dynamic Partials**: resolve partial names from the context (e.g. `{{> (*name)}}`).
- [x] Regression test for comment content colliding with variables.


## v1.4.0
- [x] Unescaped implicit iteration (`{{#list}}{{.}}{{/list}}` respects `{{{.}}}` and `{{&.}}`).
- [x] Context root may be iterated over directly.
- Additional inheritance specs covering block reindentation.
- Added Go lambda fixtures and permitted `TaggedMap` for Ruby.

## v1.4.1
- Added tests for nested partial rendering semantics.

## v1.4.2
- Interpolated content must not be re-interpolated.
- Additional dotted-name edge cases and JSON conversion tests.

## v1.4.3
- Normalised whitespace in section tests (NBSP replaced with ASCII space).
- Added Erlang lambda fixtures.

## Implementation Notes
- Core rendering engine already supports inheritance, dynamic partials and nested partials.
- Unit tests now cover interpolated content reuse, `null` value handling, dotted-name edge cases, block scope in inheritance, and implicit iteration over arrays and the context root.
- Addition of PowerShell lambda fixtures requires no changes; existing lambda tests remain valid.
- Whitespace normalisation remains to be audited.


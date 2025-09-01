# Roadmap: Mustache Spec v1.1.3 → v1.4.3

This project currently targets Mustache specification v1.4.3. The following roadmap documents the major changes from the older v1.1.3 baseline and the corresponding implementation status.

## v1.2.x
- Optional *Inheritance* module (template inheritance with `{{<parent}}` blocks).
- Extra tests for context stack behaviour.

## v1.3.x
- **Dynamic Partials**: ability to resolve partial names from context (e.g. `{{> (*name)}}`).

## v1.4.0
- Unescaped implicit iteration (`{{#list}}{{.}}{{/list}}` respects `{{{.}}}` and `{{&.}}`).
- Context root may be iterated over directly.
- Additional inheritance specs covering block reindentation.

## v1.4.1
- Nested partial rendering semantics.

## v1.4.2
- Interpolated content must not be re-interpolated.
- Additional dotted-name edge cases.

## v1.4.3
- Normalise whitespace in section tests (NBSP replaced with space).

## Implementation Notes
- Core rendering engine already supports inheritance, dynamic partials and nested partials.
- New unit test ensures interpolated content is not re-interpolated.
- Further spec compliance work should audit dotted-name behaviour and whitespace handling.

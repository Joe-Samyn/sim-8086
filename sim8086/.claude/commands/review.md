Review the following code with the rigor and perspective of Casey Muratori, Jonathan Blow, and Laurie Wired. The goal is not to flatter — the goal is to teach. Be direct, be honest, and call out anything that obscures understanding, adds unnecessary complexity, or misses an opportunity to reason clearly about the machine.

Focus on:

**Systems thinking** — Does the code reflect a clear mental model of what the hardware is actually doing? Are there missed opportunities to reason about memory layout, instruction encoding, registers, or the CPU's perspective? Call these out explicitly and explain the deeper principle.

**Simplicity and directness** — Is there unnecessary abstraction, indirection, or complexity? Would a simpler structure make the code easier to reason about? Push back on over-engineering and premature generalization.

**Naming and clarity** — Do names reflect what things actually are at the hardware/system level? Vague or misleading names are a red flag — call them out.

**Performance and efficiency** — Are there obvious inefficiencies, unnecessary copies, redundant work, or missed opportunities to use the structure of the problem? Reason about this at the level of what the CPU and memory are doing.

**Correctness and edge cases** — What can go wrong? Are there edge cases or assumptions baked in that aren't obvious? Be specific.

**What to learn** — After identifying each issue, explain the underlying principle the code is missing or violating. The goal is for the developer to walk away with a deeper understanding of systems programming, not just a list of fixes.

**Pointers, not answers** — Don't always hand the developer the solution. When the issue touches something they should discover for themselves, point them toward the right resources instead. Reference specific chapters, talks, or materials — for example:
- Casey Muratori's Handmade Hero series or his writing on clean code
- Jonathan Blow's talks on data-oriented design and language design
- The Intel 8086 manual (especially instruction encoding tables)
- Computer Organization and Design (Patterson & Hennessy)
- Code by Charles Petzold
- Specific sections of the x86 architecture manuals

When pointing to resources, be specific: chapter numbers, talk titles, timestamps if relevant. Don't just say "read the docs" — say *which* docs and *what* to look for. The goal is to push the developer to build understanding themselves, not to have it handed to them.

Be blunt. Don't soften every criticism. A good mentor tells you when something is wrong and why — they don't hedge. But also call out what's done well and why it's the right approach when you see it.

Code to review:

$ARGUMENTS

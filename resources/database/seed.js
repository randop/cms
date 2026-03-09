// Connect to the database specified by MONGO_INITDB_DATABASE
db = db.getSiblingDB("localhost");

//*****************************************************************************
// Create modes
//*****************************************************************************
db.createCollection("modes");
// Create a unique index on the 'id' field
db.modes.createIndex({ id: 1 }, { unique: true });
db.modes.insertMany([
  { id: 1, mode: "markdown" },
  { id: 2, mode: "html" },
  { id: 3, mode: "plain" },
]);

//*****************************************************************************
// Create layouts
//*****************************************************************************
db.createCollection("layouts");
const defaultHeaderValue = `<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>%REPLACE_WITH_TITLE_ID%</title>

    <!-- SEO Meta Tags -->
    <meta name="description" content="Randolph Ledesma">
    <meta name="keywords" content="Randolph Ledesma">
    <meta name="author" content="Randolph Ledesma">
    <meta name="robots" content="index, follow">

    <meta name="theme-color" content="#ffffff">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
  </head>
  <body>`;
// Create a unique index on the 'id' field
db.layouts.createIndex({ id: 1 }, { unique: true });
db.layouts.insertMany([
  { id: 1, header: defaultHeaderValue, footer: "</body></html>" },
]);

//*****************************************************************************
// Create pages
//*****************************************************************************
db.createCollection("pages");
// Create a unique index on the 'id' field
db.pages.createIndex({ id: 1 }, { unique: true });
const indexPageContent = `
<h1>Hello World</h1>
<p>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nulla ornare suscipit justo, vel tempor ligula pellentesque ac. Sed at justo lacinia, mollis urna viverra, tincidunt est. Nunc vehicula aliquam eros eu ullamcorper. Aenean placerat dui at posuere dictum. Fusce nec massa eu urna accumsan feugiat a in dolor. Suspendisse nulla elit, aliquet in nibh nec, iaculis tempor leo. Proin ipsum libero, dictum sed placerat quis, ultricies sed ante. Nullam luctus, neque at laoreet cursus, nulla ante egestas lacus, vitae rutrum orci risus sit amet lacus.</p>
<p>Etiam nibh massa, ornare vitae tristique et, tristique a lacus. Integer ac molestie erat. Donec sit amet massa dui. Proin sit amet sollicitudin ligula. Cras pretium auctor dui, sed consequat urna egestas sed. Vestibulum volutpat lorem quis tellus commodo auctor. Vivamus gravida commodo odio et mollis. Sed euismod interdum leo, bibendum volutpat sapien elementum at.</p>
`;
const aboutPageContent = `
Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur eu ultrices ligula. Maecenas id diam a elit viverra varius non quis ipsum. Donec interdum erat massa, in bibendum enim dignissim sed. Praesent viverra egestas ex id semper. Etiam molestie metus justo. Interdum et malesuada fames ac ante ipsum primis in faucibus. Phasellus maximus purus eget consequat pulvinar. Curabitur volutpat metus in erat vestibulum, vitae sollicitudin velit hendrerit. Pellentesque suscipit iaculis felis, nec volutpat enim commodo in. Aenean eu vehicula tellus, eget ullamcorper nisl. Phasellus ut arcu ac justo interdum ultricies. Duis auctor mauris in aliquet vehicula. Fusce sit amet mi diam. Vestibulum turpis velit, ultrices et luctus vel, interdum in risus. Integer metus metus, pretium nec eros maximus, laoreet tincidunt nisl. Donec eu rutrum lorem.

Vestibulum tortor odio, congue fringilla magna in, convallis molestie dui. Donec quis erat egestas, aliquet neque in, dignissim quam. Proin semper tellus at vestibulum euismod. Duis sollicitudin malesuada libero sed cursus. Fusce eget justo dolor. Etiam sodales tellus id metus ullamcorper, non molestie elit porta. Mauris aliquet sagittis sodales. Mauris vitae varius turpis. Proin hendrerit egestas pellentesque. Cras fermentum et purus in placerat. Phasellus vestibulum elit laoreet nibh tristique, non faucibus erat ultricies. Pellentesque ut fringilla sapien. Cras maximus malesuada quam quis blandit. Donec hendrerit molestie diam et consequat.

\`\`\`cpp
#include <iostream>

int main() {
    std::cout << "Hello World!!!" << std::endl;
    return 0;
}
\`\`\`

> Nulla rhoncus turpis sed purus mollis posuere. Aliquam erat volutpat. Morbi tincidunt arcu est, sit amet placerat nibh vehicula eu. Nunc eu egestas velit, eget imperdiet sem. Maecenas sed eleifend leo, sed eleifend massa. Nullam auctor ante sed diam sagittis pretium. Aliquam et sem sapien. Nulla congue risus id consequat posuere. Donec non lectus vel lorem tincidunt facilisis. Quisque leo magna, suscipit consectetur mollis eget, commodo eu dui. Phasellus feugiat malesuada diam, at elementum nunc tempus ut. Nulla tellus nunc, interdum nec efficitur ut, finibus eu mi.
`;
db.pages.insertMany([
  {
    id: "index",
    createdAt: new ISODate("2025-06-22T23:02:00Z"),
    updatedAt: new ISODate("2025-06-22T23:02:00Z"),
    modeId: 2,
    layoutId: 1,
    title: "hello",
    content: indexPageContent,
  },
  {
    id: "about",
    createdAt: new ISODate("2025-06-22T23:02:00Z"),
    updatedAt: new ISODate("2025-06-22T23:02:00Z"),
    modeId: 1,
    layoutId: 1,
    title: "About",
    content: aboutPageContent,
  },
]);

const firstPostContent = `
You’ve probably noticed it: C++ popping up more in feeds, conference talks, Reddit threads, and even casual dev conversations in late 2025 and early 2026. For a language that’s been around since the 80s and routinely called “complicated” or “unsafe,” the renewed buzz feels unexpected. Is this just nostalgia, or is something real happening?

Let’s break down what’s driving the chatter—and whether it actually matters for your next project or career move.

### The Numbers Don’t Lie: C++ Is Growing Faster Than You Think

Despite Rust’s safety hype and Python’s AI dominance, C++ developer numbers are surging. According to SlashData’s 2025 global developer trends report, the worldwide developer population jumped ~50% from ~31 million to ~47 million in just three years. Among major languages, **C++ and Rust tied for the fastest percentage growth** from 2022–2025.

Herb Sutter highlighted this late last year: there are now more C++ developers than the #1 language had four years ago. Bjarne Stroustrup cited figures around 16.3 million C++ users worldwide in 2025, with ~72% growth over four years (~20% annually). That’s millions of new (or returning) developers choosing—or sticking with—C++.

TIOBE’s monthly index (as of early 2026) still places C++ solidly in the top 5, usually trading places with C and Java right behind Python. Stack Overflow’s 2025 survey shows it used by about 24% of professional developers—down from web-heavy languages but steady where performance counts.

The surge isn’t imaginary. Software demand is outpacing hardware gains (“software taketh away faster than hardware giveth”), and C++ remains one of the few languages that delivers zero-overhead abstraction with near-metal control.

### What’s Actually Fueling the Conversation in 2025–2026?

Several things converged to make C++ feel “hot” again:

1. **Modern C++ (C++20 / C++23) finally feels mature and adopted**  
   Concepts, ranges, modules, coroutines, and better type safety landed years ago, but 2025 saw wider real-world uptake. Many teams are migrating legacy codebases or starting greenfield projects with “modern” idioms that feel far less error-prone than 2010-era C++.

2. **C++26 is coming (mid-2026 finalization)**  
   Previews promise big safety improvements: better contracts, safer relocation semantics, and tackling “erroneous behavior.” Talks at C++Now, CppCon, and ACCU in 2025 emphasized “safety, health, and efficiency” as pillars to keep C++ relevant.

3. **AI / ML infrastructure still runs on C++**  
   Python gets the glory for model training, but TensorFlow, PyTorch inference engines, CUDA backends, and most high-performance AI runtimes lean heavily on C++. As edge AI and real-time inference explode, so does demand for C++ skills.

4. **Rust envy + reality check**  
   Rust grew fast too (often neck-and-neck with C++ in percentage terms), but Rust adoption remains slower in massive legacy ecosystems, game engines, embedded, finance trading systems, browsers, and AAA games. Benchmarks show Rust and modern C++ within ~5% performance in most cases—close enough that ecosystem, hiring pool, and existing code win out for many teams.

5. **Performance hunger never went away**  
   Low-latency trading, game engines (Unreal still dominates), simulations, autonomous vehicles, aerospace, medical devices—anywhere microseconds or watts matter, C++ is often the practical choice over interpreted or GC languages.

### Should You Care? (Real Talk)

- **Yes, if you’re in (or want to enter):** game development, systems programming, embedded/IoT, high-frequency finance, HPC, graphics, robotics, or performance-critical backend services. Jobs exist, pay well, and the skill transfers broadly.
- **Maybe, if you’re curious about “close-to-metal” coding.** Modern C++ is expressive and safer than its reputation suggests—worth a weekend experiment if you like control.
- **Probably not right now, if:** you’re building web apps, quick scripts, data analysis, mobile UIs, or AI prototyping. Python, JS/TS, Go, or Rust cover those faster with less foot-gun potential.

C++ isn’t “back” because it left—it’s just that software’s insatiable appetite for speed + scale keeps pulling it forward, even as newer languages nip at its heels.

The chatter in 2025–2026? Partly hype cycles, partly genuine momentum from standards progress and raw demand in performance-critical domains.

Bottom line: C++ isn’t dying or suddenly trendy—it’s stubbornly useful. If your work lives where nanoseconds or gigabytes count, it’s worth paying attention. Otherwise, admire from afar and keep shipping in whatever gets the job done.

What about you—have you used modern C++ lately, or are you still recovering from the pre-C++11 days? Drop a thought below.
`;

const secondPostContent = `
You’ve seen the hype: “Web-scale!” “Schema-free freedom!” “Built for modern apps!” MongoDB has been the darling of startup stacks for over a decade, powering everything from MERN apps to massive enterprise backends. But in 2026, after years of real-world scars, migrations gone wrong, and stock market rollercoasters, the conversation has shifted. Here’s the unfiltered truth—pros, cons, myths, and when it’s actually the right (or wrong) choice.

### The Good Stuff That’s Still True

MongoDB delivers where it was designed to shine.

- **Developer velocity is insane at the start.** Drop a document in without defining a schema, iterate fast, prototype MVPs in days. JSON-like BSON feels native to JS/TS devs. Atlas (the cloud service) spins up clusters in minutes with auto-scaling, backups, and global distribution. Many teams swear by it for exactly this reason—getting to production quickly without fighting migrations.

- **Horizontal scaling works beautifully when you need it.** Sharding is baked in from day one. For high-write, high-read workloads with mostly independent data (logs, user sessions, content feeds, event sourcing), it outperforms many relational setups once tuned.

- **Modern features keep it relevant.** Vector search for AI/RAG apps, time-series collections, full-text search, change streams, and tight integration with AI tooling (embeddings, Voyage AI integrations in 2025) make it a strong pick for GenAI-adjacent apps. Atlas remains multi-cloud and handles data residency well.

- **Community and ecosystem are massive.** Tons of libraries, ORMs (Mongoose, Prisma with Mongo support), tutorials. It’s still #1 NoSQL in most surveys.

From 2025 Stack Overflow data, ~24% of devs use it, and satisfaction is high among those who stick with it for document-heavy workloads.

### The Ugly Truths People Whisper (or Yell on Reddit)

Here’s what gets left out of the marketing decks and bootcamp pitches.

- **Flexible schema = chaos at scale.** “Schema-on-read” sounds liberating until your team grows and no one knows what “user” actually contains. Inconsistent data shapes lead to endless defensive coding, runtime surprises, and migration nightmares when you finally need structure. Many teams end up enforcing schemas in app code anyway—why not use a DB that enforces them natively?

- **Transactions and relations are better than before—but still not free.** Multi-document ACID transactions exist since v4.0, but they’re slower and more resource-intensive than PostgreSQL equivalents. Joins? Aggregation pipelines work, but they’re verbose and can tank performance compared to SQL joins. If your data has real relationships (orders ↔ users ↔ payments), you’ll fight the model.

- **Performance is workload-dependent (and tuning is mandatory).** Without proper indexes, MongoDB can be painfully slow. Updates on large documents rewrite them entirely in some cases. Sharding helps scale out but introduces complexity—balancing, hot shards, eventual consistency gotchas. Many “MongoDB is slow” complaints trace back to poor indexing or anti-patterns.

- **You’ll probably pay more than you expect.** Atlas pricing scales with usage, and once you hit serious traffic, bills explode. Reserved capacity or committed use can help, but it’s not “cheap forever.” Recent quarters show growth deceleration in Atlas consumption, contributing to stock drops (MDB down sharply in early 2026 after guidance misses).

- **The PostgreSQL shadow is real and growing.** In 2025–2026, Postgres dominates surveys (~55-60% usage) with JSONB + pgvector closing the gap on flexibility + vectors. Many teams migrate back from MongoDB because Postgres handles mixed relational/JSON workloads better, with stronger consistency, better tooling for complex queries, and often lower long-term ops pain. Real stories: migrations taking months, but worth it for sanity.

### 2026 Reality Check: When to Use MongoDB (and When to Run)

**Use MongoDB if:**
- Your data is truly document-oriented and hierarchical (e.g., product catalogs, blogs, user profiles with nested arrays).
- You need extreme write throughput with partitionable data.
- Rapid prototyping or early-stage startup where speed-to-market > long-term maintainability.
- AI/vector search on operational data is core (Atlas Vector Search shines here).
- You already have a JS/TS-heavy team and love the ecosystem.

**Seriously consider PostgreSQL (or another SQL) instead if:**
- Data has meaningful relationships and you need complex queries/joins.
- Consistency and ACID matter a lot (finance, e-commerce transactions).
- You want one DB to rule them all (relational + JSON + vectors + time-series via extensions).
- Team size >10 and you dread “what shape is this field supposed to be?” questions.
- Long-term cost predictability and ops simplicity are priorities.

MongoDB isn’t “dead” or “bad”—it’s just not the universal hammer anymore. Postgres ate a lot of its lunch by becoming more flexible, while MongoDB stays king for pure document + scale-out use cases.

### Bottom Line

MongoDB is still excellent at what it was built for: fast iteration on evolving, semi-structured data at scale. But the “throw documents at it and figure it out later” honeymoon ends—often painfully—once you hit production scale, team growth, or real relational needs.

The honest verdict in 2026: It’s a great specialized tool, not the default choice. Pick it deliberately, not because “everyone uses MERN.” If your app looks more like a spreadsheet with relationships than a bunch of nested JSON blobs, start with Postgres and save yourself future migraines.

What’s your MongoDB story—still loving Atlas, secretly migrated away, or somewhere in between? Drop it below. No judgment.
`;

//*****************************************************************************
// Create posts
//*****************************************************************************
db.createCollection("posts");
// Create a unique index on the 'id' field
db.posts.createIndex({ id: 1 }, { unique: true });
db.posts.insertMany([
    {
        id: 1,
        createdAt: new ISODate("2026-03-09T12:17:00Z"),
        updatedAt: new ISODate("2026-03-09T12:17:00Z"),
        modeId: 1,
        layoutId: 1,
        slug: "why-everyone-talking-about-cpp",
        title: "Why Everyone Is Suddenly Talking About C++ (And Whether You Should Care)",
        content: firstPostContent,
    },
    {
        id: 2,
        createdAt: new ISODate("2026-03-09T12:17:00Z"),
        updatedAt: new ISODate("2026-03-09T12:17:00Z"),
        modeId: 1,
        layoutId: 1,
        slug: "honest-mongodb-guide",
        title: "The Honest Guide to MongoDB That No One Will Tell You",
        content: secondPostContent,
    },
]);
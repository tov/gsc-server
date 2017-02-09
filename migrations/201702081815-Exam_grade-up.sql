create table "exam_grades" (
  "id" bigserial primary key ,
  "version" integer not null,
  "user_id" bigint,
  "number" integer not null,
  "points" integer not null,
  "possible" integer not null
)

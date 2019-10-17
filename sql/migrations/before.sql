--
-- PostgreSQL database dump
--

-- Dumped from database version 11.5
-- Dumped by pg_dump version 11.5

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: assignments; Type: TABLE; Schema: public; Owner: tov
--

CREATE TABLE public.assignments (
    version integer NOT NULL,
    number integer NOT NULL,
    name text NOT NULL,
    points integer NOT NULL,
    partner boolean NOT NULL,
    web_allowed boolean NOT NULL,
    open_date timestamp without time zone,
    due_date timestamp without time zone,
    eval_date timestamp without time zone
);


ALTER TABLE public.assignments OWNER TO tov;

--
-- Name: auth_tokens; Type: TABLE; Schema: public; Owner: tov
--

CREATE TABLE public.auth_tokens (
    version integer NOT NULL,
    value text NOT NULL,
    expires timestamp without time zone,
    user_id bigint
);


ALTER TABLE public.auth_tokens OWNER TO tov;

--
-- Name: eval_items; Type: TABLE; Schema: public; Owner: tov
--

CREATE TABLE public.eval_items (
    id bigint NOT NULL,
    version integer NOT NULL,
    assignment_number integer,
    sequence integer NOT NULL,
    type integer NOT NULL,
    prompt text NOT NULL,
    relative_value double precision NOT NULL
);


ALTER TABLE public.eval_items OWNER TO tov;

--
-- Name: eval_items_id_seq; Type: SEQUENCE; Schema: public; Owner: tov
--

CREATE SEQUENCE public.eval_items_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.eval_items_id_seq OWNER TO tov;

--
-- Name: eval_items_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: tov
--

ALTER SEQUENCE public.eval_items_id_seq OWNED BY public.eval_items.id;


--
-- Name: exam_grades; Type: TABLE; Schema: public; Owner: tov
--

CREATE TABLE public.exam_grades (
    id bigint NOT NULL,
    version integer NOT NULL,
    user_id bigint,
    number integer NOT NULL,
    points integer NOT NULL,
    possible integer NOT NULL
);


ALTER TABLE public.exam_grades OWNER TO tov;

--
-- Name: exam_grades_id_seq; Type: SEQUENCE; Schema: public; Owner: tov
--

CREATE SEQUENCE public.exam_grades_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.exam_grades_id_seq OWNER TO tov;

--
-- Name: exam_grades_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: tov
--

ALTER SEQUENCE public.exam_grades_id_seq OWNED BY public.exam_grades.id;


--
-- Name: file_data; Type: TABLE; Schema: public; Owner: tov
--

CREATE TABLE public.file_data (
    version integer NOT NULL,
    file_meta_id bigint NOT NULL,
    contents bytea NOT NULL
);


ALTER TABLE public.file_data OWNER TO tov;

--
-- Name: file_meta; Type: TABLE; Schema: public; Owner: tov
--

CREATE TABLE public.file_meta (
    id bigint NOT NULL,
    version integer NOT NULL,
    name text NOT NULL,
    media_type text NOT NULL,
    purpose integer NOT NULL,
    time_stamp timestamp without time zone,
    line_count integer NOT NULL,
    byte_count integer NOT NULL,
    submission_id bigint,
    uploader_id bigint
);


ALTER TABLE public.file_meta OWNER TO tov;

--
-- Name: file_meta_id_seq; Type: SEQUENCE; Schema: public; Owner: tov
--

CREATE SEQUENCE public.file_meta_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.file_meta_id_seq OWNER TO tov;

--
-- Name: file_meta_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: tov
--

ALTER SEQUENCE public.file_meta_id_seq OWNED BY public.file_meta.id;


--
-- Name: grader_evals; Type: TABLE; Schema: public; Owner: tov
--

CREATE TABLE public.grader_evals (
    id bigint NOT NULL,
    version integer NOT NULL,
    self_eval_id bigint,
    grader_id bigint,
    content text NOT NULL,
    score double precision NOT NULL,
    time_stamp timestamp without time zone,
    status integer NOT NULL
);


ALTER TABLE public.grader_evals OWNER TO tov;

--
-- Name: grader_evals_id_seq; Type: SEQUENCE; Schema: public; Owner: tov
--

CREATE SEQUENCE public.grader_evals_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.grader_evals_id_seq OWNER TO tov;

--
-- Name: grader_evals_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: tov
--

ALTER SEQUENCE public.grader_evals_id_seq OWNED BY public.grader_evals.id;


--
-- Name: partner_requests; Type: TABLE; Schema: public; Owner: tov
--

CREATE TABLE public.partner_requests (
    id bigint NOT NULL,
    version integer NOT NULL,
    requestor_id bigint,
    requestee_id bigint,
    assignment_number integer
);


ALTER TABLE public.partner_requests OWNER TO tov;

--
-- Name: partner_requests_id_seq; Type: SEQUENCE; Schema: public; Owner: tov
--

CREATE SEQUENCE public.partner_requests_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.partner_requests_id_seq OWNER TO tov;

--
-- Name: partner_requests_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: tov
--

ALTER SEQUENCE public.partner_requests_id_seq OWNED BY public.partner_requests.id;


--
-- Name: self_evals; Type: TABLE; Schema: public; Owner: tov
--

CREATE TABLE public.self_evals (
    id bigint NOT NULL,
    version integer NOT NULL,
    eval_item_id bigint,
    submission_id bigint,
    explanation text NOT NULL,
    score double precision NOT NULL,
    permalink character varying(16) NOT NULL,
    time_stamp timestamp without time zone
);


ALTER TABLE public.self_evals OWNER TO tov;

--
-- Name: self_evals_id_seq; Type: SEQUENCE; Schema: public; Owner: tov
--

CREATE SEQUENCE public.self_evals_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.self_evals_id_seq OWNER TO tov;

--
-- Name: self_evals_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: tov
--

ALTER SEQUENCE public.self_evals_id_seq OWNED BY public.self_evals.id;


--
-- Name: submissions; Type: TABLE; Schema: public; Owner: tov
--

CREATE TABLE public.submissions (
    id bigint NOT NULL,
    version integer NOT NULL,
    user1_id bigint,
    user2_id bigint,
    assignment_number integer,
    due_date timestamp without time zone,
    eval_date timestamp without time zone,
    last_modified timestamp without time zone,
    bytes_quota integer NOT NULL
);


ALTER TABLE public.submissions OWNER TO tov;

--
-- Name: submissions_id_seq; Type: SEQUENCE; Schema: public; Owner: tov
--

CREATE SEQUENCE public.submissions_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.submissions_id_seq OWNER TO tov;

--
-- Name: submissions_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: tov
--

ALTER SEQUENCE public.submissions_id_seq OWNED BY public.submissions.id;


--
-- Name: user_stats; Type: TABLE; Schema: public; Owner: tov
--

CREATE TABLE public.user_stats (
    version integer NOT NULL,
    user_id bigint NOT NULL,
    games_played integer NOT NULL,
    score bigint NOT NULL,
    last_game timestamp without time zone
);


ALTER TABLE public.user_stats OWNER TO tov;

--
-- Name: users; Type: TABLE; Schema: public; Owner: tov
--

CREATE TABLE public.users (
    id bigint NOT NULL,
    version integer NOT NULL,
    name character varying(16) NOT NULL,
    role integer NOT NULL,
    password character varying(60) NOT NULL,
    password_method character varying(6) NOT NULL,
    password_salt character varying(16) NOT NULL,
    failed_login_attempts integer NOT NULL,
    last_login_attempt timestamp without time zone
);


ALTER TABLE public.users OWNER TO tov;

--
-- Name: users_id_seq; Type: SEQUENCE; Schema: public; Owner: tov
--

CREATE SEQUENCE public.users_id_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


ALTER TABLE public.users_id_seq OWNER TO tov;

--
-- Name: users_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: tov
--

ALTER SEQUENCE public.users_id_seq OWNED BY public.users.id;


--
-- Name: eval_items id; Type: DEFAULT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.eval_items ALTER COLUMN id SET DEFAULT nextval('public.eval_items_id_seq'::regclass);


--
-- Name: exam_grades id; Type: DEFAULT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.exam_grades ALTER COLUMN id SET DEFAULT nextval('public.exam_grades_id_seq'::regclass);


--
-- Name: file_meta id; Type: DEFAULT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.file_meta ALTER COLUMN id SET DEFAULT nextval('public.file_meta_id_seq'::regclass);


--
-- Name: grader_evals id; Type: DEFAULT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.grader_evals ALTER COLUMN id SET DEFAULT nextval('public.grader_evals_id_seq'::regclass);


--
-- Name: partner_requests id; Type: DEFAULT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.partner_requests ALTER COLUMN id SET DEFAULT nextval('public.partner_requests_id_seq'::regclass);


--
-- Name: self_evals id; Type: DEFAULT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.self_evals ALTER COLUMN id SET DEFAULT nextval('public.self_evals_id_seq'::regclass);


--
-- Name: submissions id; Type: DEFAULT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.submissions ALTER COLUMN id SET DEFAULT nextval('public.submissions_id_seq'::regclass);


--
-- Name: users id; Type: DEFAULT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.users ALTER COLUMN id SET DEFAULT nextval('public.users_id_seq'::regclass);


--
-- Data for Name: assignments; Type: TABLE DATA; Schema: public; Owner: tov
--

COPY public.assignments (version, number, name, points, partner, web_allowed, open_date, due_date, eval_date) FROM stdin;
0	1	Homework 1	10	t	t	2019-09-23 21:45:14.995	2019-09-30 21:45:14.995	2019-10-02 21:45:14.995
0	2	Homework 2	10	t	t	2019-09-30 21:45:14.995	2019-10-07 21:45:14.995	2019-10-09 21:45:14.995
0	3	Homework 3	10	t	t	2019-09-25 21:45:14.995	2019-10-02 21:45:14.995	2019-10-04 21:45:14.995
0	4	Homework 4	10	t	t	2019-10-07 21:45:14.995	2019-10-14 21:45:14.995	2019-10-16 21:45:14.995
\.


--
-- Data for Name: auth_tokens; Type: TABLE DATA; Schema: public; Owner: tov
--

COPY public.auth_tokens (version, value, expires, user_id) FROM stdin;
1	YmhZbDhckftduGX3jZ4GSQ==	2019-10-20 03:13:10.29	2
\.


--
-- Data for Name: eval_items; Type: TABLE DATA; Schema: public; Owner: tov
--

COPY public.eval_items (id, version, assignment_number, sequence, type, prompt, relative_value) FROM stdin;
\.


--
-- Data for Name: exam_grades; Type: TABLE DATA; Schema: public; Owner: tov
--

COPY public.exam_grades (id, version, user_id, number, points, possible) FROM stdin;
1	0	3	1	40	50
2	0	3	2	37	50
3	0	4	1	40	50
4	0	4	2	37	50
5	0	5	1	40	50
6	0	5	2	37	50
7	0	6	1	40	50
8	0	6	2	37	50
\.


--
-- Data for Name: file_data; Type: TABLE DATA; Schema: public; Owner: tov
--

COPY public.file_data (version, file_meta_id, contents) FROM stdin;
0	1	\\x23707261676d61206f6e63650a
0	2	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	3	\\x23707261676d61206f6e63650a
0	4	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	5	\\x23707261676d61206f6e63650a
0	6	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	7	\\x23707261676d61206f6e63650a
0	8	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	9	\\x23707261676d61206f6e63650a
0	10	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	11	\\x23707261676d61206f6e63650a
0	12	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	13	\\x23707261676d61206f6e63650a
0	14	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	15	\\x23707261676d61206f6e63650a
0	16	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	17	\\x23707261676d61206f6e63650a
0	18	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	19	\\x23707261676d61206f6e63650a
0	20	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	21	\\x23707261676d61206f6e63650a
0	22	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	23	\\x23707261676d61206f6e63650a
0	24	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	25	\\x23707261676d61206f6e63650a
0	26	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	27	\\x23707261676d61206f6e63650a
0	28	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	29	\\x23707261676d61206f6e63650a
0	30	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
0	31	\\x23707261676d61206f6e63650a
0	32	\\x23696e636c756465202266696c652e68220a0a6e616d657370616365206d6568207b0a0a7d0a
\.


--
-- Data for Name: file_meta; Type: TABLE DATA; Schema: public; Owner: tov
--

COPY public.file_meta (id, version, name, media_type, purpose, time_stamp, line_count, byte_count, submission_id, uploader_id) FROM stdin;
1	0	file.h	text/plain	0	2019-10-03 21:45:15.018	1	13	1	3
2	0	file.C	text/plain	0	2019-10-03 21:45:15.019	5	38	1	3
3	0	file.h	text/plain	0	2019-10-03 21:45:15.02	1	13	2	3
4	0	file.C	text/plain	0	2019-10-03 21:45:15.021	5	38	2	3
5	0	file.h	text/plain	0	2019-10-03 21:45:15.021	1	13	3	3
6	0	file.C	text/plain	0	2019-10-03 21:45:15.022	5	38	3	3
7	0	file.h	text/plain	0	2019-10-03 21:45:15.023	1	13	4	3
8	0	file.C	text/plain	0	2019-10-03 21:45:15.023	5	38	4	3
9	0	file.h	text/plain	0	2019-10-03 21:45:15.031	1	13	5	4
10	0	file.C	text/plain	0	2019-10-03 21:45:15.032	5	38	5	4
11	0	file.h	text/plain	0	2019-10-03 21:45:15.032	1	13	6	4
12	0	file.C	text/plain	0	2019-10-03 21:45:15.033	5	38	6	4
13	0	file.h	text/plain	0	2019-10-03 21:45:15.033	1	13	7	4
14	0	file.C	text/plain	0	2019-10-03 21:45:15.034	5	38	7	4
15	0	file.h	text/plain	0	2019-10-03 21:45:15.034	1	13	8	4
16	0	file.C	text/plain	0	2019-10-03 21:45:15.035	5	38	8	4
17	0	file.h	text/plain	0	2019-10-03 21:45:15.042	1	13	9	5
18	0	file.C	text/plain	0	2019-10-03 21:45:15.042	5	38	9	5
19	0	file.h	text/plain	0	2019-10-03 21:45:15.043	1	13	10	5
20	0	file.C	text/plain	0	2019-10-03 21:45:15.043	5	38	10	5
21	0	file.h	text/plain	0	2019-10-03 21:45:15.044	1	13	11	5
22	0	file.C	text/plain	0	2019-10-03 21:45:15.044	5	38	11	5
23	0	file.h	text/plain	0	2019-10-03 21:45:15.045	1	13	12	5
24	0	file.C	text/plain	0	2019-10-03 21:45:15.058	5	38	12	5
25	0	file.h	text/plain	0	2019-10-03 21:45:15.066	1	13	13	6
26	0	file.C	text/plain	0	2019-10-03 21:45:15.067	5	38	13	6
27	0	file.h	text/plain	0	2019-10-03 21:45:15.068	1	13	14	6
28	0	file.C	text/plain	0	2019-10-03 21:45:15.069	5	38	14	6
29	0	file.h	text/plain	0	2019-10-03 21:45:15.071	1	13	15	6
30	0	file.C	text/plain	0	2019-10-03 21:45:15.072	5	38	15	6
31	0	file.h	text/plain	0	2019-10-03 21:45:15.073	1	13	16	6
32	0	file.C	text/plain	0	2019-10-03 21:45:15.074	5	38	16	6
\.


--
-- Data for Name: grader_evals; Type: TABLE DATA; Schema: public; Owner: tov
--

COPY public.grader_evals (id, version, self_eval_id, grader_id, content, score, time_stamp, status) FROM stdin;
\.


--
-- Data for Name: partner_requests; Type: TABLE DATA; Schema: public; Owner: tov
--

COPY public.partner_requests (id, version, requestor_id, requestee_id, assignment_number) FROM stdin;
\.


--
-- Data for Name: self_evals; Type: TABLE DATA; Schema: public; Owner: tov
--

COPY public.self_evals (id, version, eval_item_id, submission_id, explanation, score, permalink, time_stamp) FROM stdin;
\.


--
-- Data for Name: submissions; Type: TABLE DATA; Schema: public; Owner: tov
--

COPY public.submissions (id, version, user1_id, user2_id, assignment_number, due_date, eval_date, last_modified, bytes_quota) FROM stdin;
1	0	3	\N	1	2019-09-30 21:45:14.995	2019-10-02 21:45:14.995	2019-10-03 21:45:15.019	20971520
2	0	3	\N	2	2019-10-07 21:45:14.995	2019-10-09 21:45:14.995	2019-10-03 21:45:15.021	20971520
3	0	3	\N	3	2019-10-02 21:45:14.995	2019-10-04 21:45:14.995	2019-10-03 21:45:15.022	20971520
4	0	3	\N	4	2019-10-14 21:45:14.995	2019-10-16 21:45:14.995	2019-10-03 21:45:15.023	20971520
5	0	4	\N	1	2019-09-30 21:45:14.995	2019-10-02 21:45:14.995	2019-10-03 21:45:15.032	20971520
6	0	4	\N	2	2019-10-07 21:45:14.995	2019-10-09 21:45:14.995	2019-10-03 21:45:15.033	20971520
7	0	4	\N	3	2019-10-02 21:45:14.995	2019-10-04 21:45:14.995	2019-10-03 21:45:15.034	20971520
8	0	4	\N	4	2019-10-14 21:45:14.995	2019-10-16 21:45:14.995	2019-10-03 21:45:15.035	20971520
9	0	5	\N	1	2019-09-30 21:45:14.995	2019-10-02 21:45:14.995	2019-10-03 21:45:15.042	20971520
10	0	5	\N	2	2019-10-07 21:45:14.995	2019-10-09 21:45:14.995	2019-10-03 21:45:15.043	20971520
11	0	5	\N	3	2019-10-02 21:45:14.995	2019-10-04 21:45:14.995	2019-10-03 21:45:15.044	20971520
12	0	5	\N	4	2019-10-14 21:45:14.995	2019-10-16 21:45:14.995	2019-10-03 21:45:15.058	20971520
13	0	6	\N	1	2019-09-30 21:45:14.995	2019-10-02 21:45:14.995	2019-10-03 21:45:15.067	20971520
14	0	6	\N	2	2019-10-07 21:45:14.995	2019-10-09 21:45:14.995	2019-10-03 21:45:15.069	20971520
15	0	6	\N	3	2019-10-02 21:45:14.995	2019-10-04 21:45:14.995	2019-10-03 21:45:15.072	20971520
16	0	6	\N	4	2019-10-14 21:45:14.995	2019-10-16 21:45:14.995	2019-10-03 21:45:15.074	20971520
\.


--
-- Data for Name: user_stats; Type: TABLE DATA; Schema: public; Owner: tov
--

COPY public.user_stats (version, user_id, games_played, score, last_game) FROM stdin;
\.


--
-- Data for Name: users; Type: TABLE DATA; Schema: public; Owner: tov
--

COPY public.users (id, version, name, role, password, password_method, password_salt, failed_login_attempts, last_login_attempt) FROM stdin;
1	0	root	2	$2y$07$Qyn1P1isbhPGKzXrYTXwLOd3hU3jxs2koeQgAcn3nQWExqpM33956	bcrypt	KJwGy.v4H3VmiVr5	0	\N
3	0	student	0	$2y$07$OVfhMSDVOk3wRTn0KjnAQ.X9Q2wZ.pYJIIE7xZ3N413hHXWAAJcnO	bcrypt	Axc9AWBnrMZv2ZBH	0	\N
4	0	s1	0	$2y$07$RUHjKDXiJizNbSjjQin4beStgP4W/PKjz7GIqD23ZoTb/upCx3QK6	bcrypt	Mbe0Vd.MOuIeJJzv	0	\N
5	0	s2	0	$2y$07$bxXJLj/xOkXVRBWxZ07sTOCW03aAPKfLUtHWD2Zl3q9F6RX2VjKqW	bcrypt	w6K6PsBfWL63oonU	0	\N
6	0	s3	0	$2y$07$UTW2RzbGYlf3PEe2XjbJQe4TjR0OuUzsYSF/Rf/u3jTccaI5ZV9e6	bcrypt	YV8OWHjxyDh8fWKJ	0	\N
2	5	jtov	2	$2y$07$TjTQMFHBYkv2XTX1PiznZuxWB/9h3iT4ortyz/T5UogcvzXktUfea	bcrypt	VUR8rCjlxeVwFMio	0	2019-10-06 03:13:10.276
\.


--
-- Name: eval_items_id_seq; Type: SEQUENCE SET; Schema: public; Owner: tov
--

SELECT pg_catalog.setval('public.eval_items_id_seq', 1, false);


--
-- Name: exam_grades_id_seq; Type: SEQUENCE SET; Schema: public; Owner: tov
--

SELECT pg_catalog.setval('public.exam_grades_id_seq', 8, true);


--
-- Name: file_meta_id_seq; Type: SEQUENCE SET; Schema: public; Owner: tov
--

SELECT pg_catalog.setval('public.file_meta_id_seq', 32, true);


--
-- Name: grader_evals_id_seq; Type: SEQUENCE SET; Schema: public; Owner: tov
--

SELECT pg_catalog.setval('public.grader_evals_id_seq', 1, false);


--
-- Name: partner_requests_id_seq; Type: SEQUENCE SET; Schema: public; Owner: tov
--

SELECT pg_catalog.setval('public.partner_requests_id_seq', 1, false);


--
-- Name: self_evals_id_seq; Type: SEQUENCE SET; Schema: public; Owner: tov
--

SELECT pg_catalog.setval('public.self_evals_id_seq', 1, false);


--
-- Name: submissions_id_seq; Type: SEQUENCE SET; Schema: public; Owner: tov
--

SELECT pg_catalog.setval('public.submissions_id_seq', 16, true);


--
-- Name: users_id_seq; Type: SEQUENCE SET; Schema: public; Owner: tov
--

SELECT pg_catalog.setval('public.users_id_seq', 6, true);


--
-- Name: assignments assignments_pkey; Type: CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.assignments
    ADD CONSTRAINT assignments_pkey PRIMARY KEY (number);


--
-- Name: auth_tokens auth_tokens_pkey; Type: CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.auth_tokens
    ADD CONSTRAINT auth_tokens_pkey PRIMARY KEY (value);


--
-- Name: eval_items eval_items_pkey; Type: CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.eval_items
    ADD CONSTRAINT eval_items_pkey PRIMARY KEY (id);


--
-- Name: exam_grades exam_grades_pkey; Type: CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.exam_grades
    ADD CONSTRAINT exam_grades_pkey PRIMARY KEY (id);


--
-- Name: file_data file_data_pkey; Type: CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.file_data
    ADD CONSTRAINT file_data_pkey PRIMARY KEY (file_meta_id);


--
-- Name: file_meta file_meta_pkey; Type: CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.file_meta
    ADD CONSTRAINT file_meta_pkey PRIMARY KEY (id);


--
-- Name: grader_evals grader_evals_pkey; Type: CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.grader_evals
    ADD CONSTRAINT grader_evals_pkey PRIMARY KEY (id);


--
-- Name: partner_requests partner_requests_pkey; Type: CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.partner_requests
    ADD CONSTRAINT partner_requests_pkey PRIMARY KEY (id);


--
-- Name: self_evals self_evals_pkey; Type: CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.self_evals
    ADD CONSTRAINT self_evals_pkey PRIMARY KEY (id);


--
-- Name: submissions submissions_pkey; Type: CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.submissions
    ADD CONSTRAINT submissions_pkey PRIMARY KEY (id);


--
-- Name: user_stats user_stats_pkey; Type: CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.user_stats
    ADD CONSTRAINT user_stats_pkey PRIMARY KEY (user_id);


--
-- Name: users users_pkey; Type: CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.users
    ADD CONSTRAINT users_pkey PRIMARY KEY (id);


--
-- Name: ix_self_evals_permalink; Type: INDEX; Schema: public; Owner: tov
--

CREATE INDEX ix_self_evals_permalink ON public.self_evals USING btree (permalink);


--
-- Name: ix_users_name; Type: INDEX; Schema: public; Owner: tov
--

CREATE INDEX ix_users_name ON public.users USING btree (name);


--
-- Name: auth_tokens fk_auth_tokens_user; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.auth_tokens
    ADD CONSTRAINT fk_auth_tokens_user FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED;


--
-- Name: eval_items fk_eval_items_assignment; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.eval_items
    ADD CONSTRAINT fk_eval_items_assignment FOREIGN KEY (assignment_number) REFERENCES public.assignments(number) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED;


--
-- Name: exam_grades fk_exam_grades_user; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.exam_grades
    ADD CONSTRAINT fk_exam_grades_user FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED;


--
-- Name: file_data fk_file_data_file_meta; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.file_data
    ADD CONSTRAINT fk_file_data_file_meta FOREIGN KEY (file_meta_id) REFERENCES public.file_meta(id) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED;


--
-- Name: file_meta fk_file_meta_submission; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.file_meta
    ADD CONSTRAINT fk_file_meta_submission FOREIGN KEY (submission_id) REFERENCES public.submissions(id) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED;


--
-- Name: file_meta fk_file_meta_uploader; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.file_meta
    ADD CONSTRAINT fk_file_meta_uploader FOREIGN KEY (uploader_id) REFERENCES public.users(id) ON DELETE SET NULL DEFERRABLE INITIALLY DEFERRED;


--
-- Name: grader_evals fk_grader_evals_grader; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.grader_evals
    ADD CONSTRAINT fk_grader_evals_grader FOREIGN KEY (grader_id) REFERENCES public.users(id) ON DELETE SET NULL DEFERRABLE INITIALLY DEFERRED;


--
-- Name: grader_evals fk_grader_evals_self_eval; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.grader_evals
    ADD CONSTRAINT fk_grader_evals_self_eval FOREIGN KEY (self_eval_id) REFERENCES public.self_evals(id) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED;


--
-- Name: partner_requests fk_partner_requests_assignment; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.partner_requests
    ADD CONSTRAINT fk_partner_requests_assignment FOREIGN KEY (assignment_number) REFERENCES public.assignments(number) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED;


--
-- Name: partner_requests fk_partner_requests_requestee; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.partner_requests
    ADD CONSTRAINT fk_partner_requests_requestee FOREIGN KEY (requestee_id) REFERENCES public.users(id) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED;


--
-- Name: partner_requests fk_partner_requests_requestor; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.partner_requests
    ADD CONSTRAINT fk_partner_requests_requestor FOREIGN KEY (requestor_id) REFERENCES public.users(id) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED;


--
-- Name: self_evals fk_self_evals_eval_item; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.self_evals
    ADD CONSTRAINT fk_self_evals_eval_item FOREIGN KEY (eval_item_id) REFERENCES public.eval_items(id) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED;


--
-- Name: self_evals fk_self_evals_submission; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.self_evals
    ADD CONSTRAINT fk_self_evals_submission FOREIGN KEY (submission_id) REFERENCES public.submissions(id) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED;


--
-- Name: submissions fk_submissions_assignment; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.submissions
    ADD CONSTRAINT fk_submissions_assignment FOREIGN KEY (assignment_number) REFERENCES public.assignments(number) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED;


--
-- Name: submissions fk_submissions_user1; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.submissions
    ADD CONSTRAINT fk_submissions_user1 FOREIGN KEY (user1_id) REFERENCES public.users(id) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED;


--
-- Name: submissions fk_submissions_user2; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.submissions
    ADD CONSTRAINT fk_submissions_user2 FOREIGN KEY (user2_id) REFERENCES public.users(id) ON DELETE SET NULL DEFERRABLE INITIALLY DEFERRED;


--
-- Name: user_stats fk_user_stats_user; Type: FK CONSTRAINT; Schema: public; Owner: tov
--

ALTER TABLE ONLY public.user_stats
    ADD CONSTRAINT fk_user_stats_user FOREIGN KEY (user_id) REFERENCES public.users(id) ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED;


--
-- PostgreSQL database dump complete
--


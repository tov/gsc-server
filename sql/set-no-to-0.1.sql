UPDATE grader_evals
SET score = 0.1
WHERE id = ANY(SELECT g.id
               FROM grader_evals g
               INNER JOIN self_evals s ON g.self_eval_id = s.id
               INNER JOIN eval_items e ON s.eval_item_id = e.id
               WHERE e.type = 0
                 AND s.score = 0
                 AND g.score = 0);

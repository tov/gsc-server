#include "model/auth/User.h"
#include "model/File_data.h"
#include "model/File_meta.h"
#include "model/Session.h"
#include "model/Submission.h"

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Postgres>

#include <iostream>
#include <cstdlib>

namespace dbo = Wt::Dbo;

int main(int argc, const char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " HWNUMBER USERNAME\n";
        std::exit(1);
    }

    int hw_number = std::atoi(argv[1]);
    std::string username(argv[2]);

    auto db_string = std::getenv("POSTGRES_CONNINFO");
    dbo::backend::Postgres connection(db_string ? db_string : "dbname=gsc");
    dbo::Session dbo;
    dbo.setConnection(connection);
    Session::map_classes(dbo);

    dbo::Transaction transaction(dbo);

    dbo::ptr<User> user = dbo.find<User>().where("name = ?").bind(username);

    if (!user) {
        std::cerr << "User not found: " << username << '\n';
        std::exit(2);
    }

    dbo::ptr<Submission> submission =
            dbo.find<Submission>()
               .where("assignment_number = ?")
               .bind(hw_number)
               .where("(user1_id = ? OR user2_id = ?)")
               .bind(user.id()).bind(user.id());

    if (!submission) {
        std::cerr << "Submission not found: " << hw_number << '\n';
        std::exit(3);
    }

    for (auto file : submission->source_files()) {
        const auto& text = file->file_data()->contents();
        std::cout << "Writing " << file->name() << "...";
        std::ofstream of(file->name(), std::ios::binary);
        of.write(text.data(), text.size());
        std::cout << "done\n";
    }
}

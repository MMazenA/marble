#!/usr/bin/env bash
set -euo pipefail

RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[1;33m"
BLUE="\033[0;34m"
CYAN="\033[0;36m"
NC="\033[0m"

indent() { sed 's/^/    /'; }

CONN_STR="host=localhost port=5432 dbname=marble user=user password=password"
PATTERN='stg_%'
ALWAYS_DROP="schema_migrations"

echo -e "${BLUE}Checking for objects matching '${CYAN}${PATTERN}${BLUE}'...${NC}"

TABLES=$(psql "$CONN_STR" -At -c "
    SELECT tablename
    FROM pg_tables
    WHERE schemaname='public'
      AND tablename LIKE '${PATTERN}'
")

VIEWS=$(psql "$CONN_STR" -At -c "
    SELECT viewname
    FROM pg_views
    WHERE schemaname='public'
      AND viewname LIKE '${PATTERN}'
")

FUNCTIONS=$(psql "$CONN_STR" -At -c "
    SELECT p.oid::regprocedure
    FROM pg_proc p
    WHERE p.pronamespace = 'public'::regnamespace
      AND p.proname LIKE '${PATTERN}'
")

SEQUENCES=$(psql "$CONN_STR" -At -c "
    SELECT sequencename
    FROM pg_sequences
    WHERE schemaname='public'
      AND sequencename LIKE '${PATTERN}'
")

echo -e "\n${YELLOW}Objects that will be dropped:${NC}"
echo -e "  - ${RED}${ALWAYS_DROP}${NC} (forced)"

[[ -n "$TABLES" ]] && echo -e "${YELLOW}Tables:${NC}" && (while IFS= read -r tbl; do [[ -n "$tbl" ]] && printf "  - ${RED}%s${NC}\n" "$tbl"; done <<< "$TABLES")

[[ -n "$VIEWS" ]] && echo -e "${YELLOW}Views:${NC}" && (while IFS= read -r view; do [[ -n "$view" ]] && printf "  - ${RED}%s${NC}\n" "$view"; done <<< "$VIEWS")

[[ -n "$FUNCTIONS" ]] && echo -e "${YELLOW}Functions:${NC}" && (while IFS= read -r func; do [[ -n "$func" ]] && printf "  - ${RED}%s${NC}\n" "$func"; done <<< "$FUNCTIONS")

[[ -n "$SEQUENCES" ]] && echo -e "${YELLOW}Sequences:${NC}" && (while IFS= read -r seq; do [[ -n "$seq" ]] && printf "  - ${RED}%s${NC}\n" "$seq"; done <<< "$SEQUENCES")

echo

read -r -p "$(echo -e "${YELLOW}Proceed with DROP on all these objects? (y/N): ${NC}")" confirm
if [[ "$confirm" != "y" ]]; then
  echo -e "${RED}Cancelled.${NC}"
  exit 1
fi

echo -e "\n${RED}Dropping schema_migrations...${NC}"
psql "$CONN_STR" -c "DROP TABLE IF EXISTS ${ALWAYS_DROP} CASCADE;" 2>&1 | indent

echo -e "${RED}Dropping views matching pattern...${NC}"
psql "$CONN_STR" -c "
DO \$\$
DECLARE r RECORD;
BEGIN
  FOR r IN SELECT viewname FROM pg_views
           WHERE schemaname='public'
             AND viewname LIKE '${PATTERN}'
  LOOP
    EXECUTE 'DROP VIEW IF EXISTS ' || quote_ident(r.viewname) || ' CASCADE';
    RAISE NOTICE 'Dropped view: %', r.viewname;
  END LOOP;
END
\$\$;
" 2>&1 | indent

echo -e "${RED}Dropping functions matching pattern...${NC}"
psql "$CONN_STR" -c "
DO \$\$
DECLARE r RECORD;
BEGIN
  FOR r IN SELECT p.oid::regprocedure as proc
           FROM pg_proc p
           WHERE p.pronamespace = 'public'::regnamespace
             AND p.proname LIKE '${PATTERN}'
  LOOP
    EXECUTE 'DROP FUNCTION IF EXISTS ' || r.proc || ' CASCADE';
    RAISE NOTICE 'Dropped function: %', r.proc;
  END LOOP;
END
\$\$;
" 2>&1 | indent

echo -e "${RED}Dropping sequences matching pattern...${NC}"
psql "$CONN_STR" -c "
DO \$\$
DECLARE r RECORD;
BEGIN
  FOR r IN SELECT sequencename FROM pg_sequences
           WHERE schemaname='public'
             AND sequencename LIKE '${PATTERN}'
  LOOP
    EXECUTE 'DROP SEQUENCE IF EXISTS ' || quote_ident(r.sequencename) || ' CASCADE';
    RAISE NOTICE 'Dropped sequence: %', r.sequencename;
  END LOOP;
END
\$\$;
" 2>&1 | indent

echo -e "${RED}Dropping pattern-matched staging tables...${NC}"
psql "$CONN_STR" -c "
DO \$\$
DECLARE r RECORD;
BEGIN
  FOR r IN SELECT tablename FROM pg_tables
           WHERE schemaname='public'
             AND tablename LIKE '${PATTERN}'
  LOOP
    EXECUTE 'DROP TABLE IF EXISTS ' || quote_ident(r.tablename) || ' CASCADE';
    RAISE NOTICE 'Dropped table: %', r.tablename;
  END LOOP;
END
\$\$;
" 2>&1 | indent

echo -e "${GREEN}Done.${NC}\n"

read -r -p "$(echo -e "${YELLOW}Run quarry_bootstrap? (y/N): ${NC}")" confirm_bootstrap
if [[ "$confirm_bootstrap" == "y" ]]; then
  echo -e "${GREEN}Running quarry_bootstrap...${NC}"
  ./build/quarry/quarry_bootstrap 2>&1 | indent
fi

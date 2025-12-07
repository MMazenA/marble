#!/usr/bin/env bash
set -euo pipefail

RED="\033[0;31m"
GREEN="\033[0;32m"
YELLOW="\033[1;33m"
BLUE="\033[0;34m"
CYAN="\033[0;36m"
NC="\033[0m"

indent() { sed 's/^/    /'; }

CONN_STR="host=localhost port=5432 dbname=postgres user=user password=password"
PATTERN='stg_%'
ALWAYS_DROP="schema_migrations"

echo -e "${BLUE}Checking for tables matching '${CYAN}${PATTERN}${BLUE}'...${NC}"

TABLES=$(psql "$CONN_STR" -At -c "
    SELECT tablename
    FROM pg_tables
    WHERE schemaname='public'
      AND tablename LIKE '${PATTERN}'
")

echo -e "\n${YELLOW}Tables that will be dropped:${NC}"
echo -e "  - ${RED}${ALWAYS_DROP}${NC} (forced)"

while IFS= read -r tbl; do
  [[ -n "$tbl" ]] && printf "  - ${RED}%s${NC}\n" "$tbl"
done <<< "$TABLES"

echo

read -r -p "$(echo -e "${YELLOW}Proceed with DROP TABLE on these tables? (y/N): ${NC}")" confirm
if [[ "$confirm" != "y" ]]; then
  echo -e "${RED}Cancelled.${NC}"
  exit 1
fi

echo -e "\n${RED}Dropping schema_migrations...${NC}"
psql "$CONN_STR" -c "DROP TABLE IF EXISTS ${ALWAYS_DROP} CASCADE;" 2>&1 | indent

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
    RAISE NOTICE 'Dropped: %', r.tablename;
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

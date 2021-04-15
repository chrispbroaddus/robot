package data

import (
	"bytes"
	"time"
)

// User contains all of the auth information and the users ID
type User struct {
	UserID string
	Admin  bool
	UserAuth
}

// UserAuth contains all of the possible auth information for a user
type UserAuth struct {
	UserName string
	Password string
}

// CreateUserAuth is the PersistentStorage implementation to create a user record with login info
func (p *PersistentStorage) CreateUserAuth(userID, userName, password string) (*User, error) {
	defer func(now time.Time) {
		requestTime.With("method", createUserAuth, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var stmt bytes.Buffer
	stmt.WriteString("WITH u as (INSERT INTO users(id, user_name) VALUES($1, $2) RETURNING id, user_name) ")
	stmt.WriteString("auth as (INSERT INTO users_auth(user_id, password) VALUES($1, $3) RETURNING password, is_admin)")
	stmt.WriteString("SELECT u.id, u.user_name, auth.password, auth.is_admin ")
	stmt.WriteString("FROM u, auth")

	row := p.db.QueryRow(stmt.String(), userID, userName, password)

	user := new(User)
	err := row.Scan(&user.UserID, &user.UserName, &user.Password, &user.Admin)
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", createUserAuth, "data_store", postgres).Add(1)
		return nil, err
	}

	return user, nil
}

// UpdateUserAuth is the PersistentStorage implementation to update a users username and or password
func (p *PersistentStorage) UpdateUserAuth(userID, userName, password string) (*User, error) {
	defer func(now time.Time) {
		requestTime.With("method", updateUserAuth, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var stmt bytes.Buffer
	stmt.WriteString("WITH u as ( UPDATE users SET user_name=$2  WHERE id=$1 RETURNING id, user_name ) ")
	stmt.WriteString("auth as (UPDATE users_auth auth SET password=$3 WHERE user_id=$1 RETURNING password, is_admin) ")
	stmt.WriteString("SELECT u.id, u.user_name, auth.password, auth.is_admin ")
	stmt.WriteString("FROM u, auth")

	row := p.db.QueryRow(stmt.String(), userID, userName, password)

	user := new(User)
	err := row.Scan(&user.UserID, &user.UserName, &user.Password, &user.Admin)
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", updateUserAuth, "data_store", postgres).Add(1)
		return nil, err
	}

	return user, nil
}

// SetAdmin is the PersistentStorage implementation to set a users admin status
func (p *PersistentStorage) SetAdmin(userID string, isAdmin bool) error {
	defer func(now time.Time) {
		requestTime.With("method", setAdmin, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var stmt bytes.Buffer
	stmt.WriteString("UPDATE users_auth SET is_admin=$2 ")
	stmt.WriteString("WHERE user_id=$1")

	_, err := p.db.Exec(stmt.String(), userID, isAdmin)
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", setAdmin, "data_store", postgres).Add(1)
		return err
	}

	return nil
}

// FindUser is the PersistentStorage implementation to get a user by id
func (p *PersistentStorage) FindUser(userID string) (*User, error) {
	defer func(now time.Time) {
		requestTime.With("method", findUser, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var stmt bytes.Buffer
	stmt.WriteString("SELECT u.id, u.user_name, auth.is_admin, auth.password ")
	stmt.WriteString("FROM users as u, users_auth as auth ")
	stmt.WriteString("WHERE u.id=$1 AND auth.user_id=$1")

	row := p.db.QueryRow(stmt.String(), userID)

	user := new(User)
	err := row.Scan(&user.UserID, &user.UserName, &user.Admin, &user.Password)
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", findUser, "data_store", postgres).Add(1)
		return nil, err
	}

	return user, nil
}

// FindUserByName is the PersistentStorage implementation to get a user by username
func (p *PersistentStorage) FindUserByName(userName string) (*User, error) {
	defer func(now time.Time) {
		requestTime.With("method", findUserByName, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var stmt bytes.Buffer
	stmt.WriteString("SELECT u.id, u.user_name, auth.is_admin, auth.password ")
	stmt.WriteString("FROM users u JOIN users_auth auth ON u.id = auth.user_id ")
	stmt.WriteString("WHERE u.user_name=$1")

	row := p.db.QueryRow(stmt.String(), userName)

	user := new(User)
	err := row.Scan(&user.UserID, &user.UserName, &user.Admin, &user.Password)
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", findUserByName, "data_store", postgres).Add(1)
		return nil, err
	}

	return user, nil
}

// CreateUserAuth is the localStorage implementation to create a user record with login info
func (l *localStorage) CreateUserAuth(userID, userName, password string) (*User, error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	newUser := &User{
		UserID: userID,
		UserAuth: UserAuth{
			UserName: userName,
			Password: password,
		},
	}
	l.users[userID] = newUser

	return newUser, nil
}

// UpdateUserAuth is the localStorage implementation to update a users username and or password
func (l *localStorage) UpdateUserAuth(userID, userName, password string) (*User, error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	user, ok := l.users[userID]
	if !ok {
		return nil, ErrNoUser
	}

	user.UserName = userName
	user.Password = password
	l.users[userID] = user

	return user, nil
}

// SetAdmin is the localStorage implementation to set a users admin status
func (l *localStorage) SetAdmin(userID string, isAdmin bool) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	user, ok := l.users[userID]
	if !ok {
		return ErrNoUser
	}

	user.Admin = isAdmin
	l.users[userID] = user

	return nil
}

// FindUser the user from the local storage by their ID
func (l *localStorage) FindUser(userID string) (*User, error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	user, ok := l.users[userID]
	if !ok {
		return nil, ErrNoUser
	}

	return user, nil
}

// FindUserByName get the user from local storage that has the same userName
func (l *localStorage) FindUserByName(userName string) (*User, error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	for _, user := range l.users {
		if user.UserName == userName {
			return user, nil
		}
	}

	return nil, ErrNoUser
}
